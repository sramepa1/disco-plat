#include "Network.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cstring>
#include <typeinfo>

#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <unistd.h>

#include "../build/Interface.h"
#include "NeighbourIface.h"
#include "NeighbourImpl.h"
#include "Synchronization.h"
#include "Repository.h"
#include "globals.h"

using namespace std;
using namespace disco_plat;
using namespace CORBA;
using namespace PortableServer;

#define BIND_AND_ASSIGN(objectIDL, addr, var, type) \
    tempObj = orb->bind(objectIDL, addr); \
    if (is_nil(tempObj)) { \
        cerr << "Node (port " << myID.identifier << ") - cannot bind to " << addr << endl; \
        throw "Cannot bind!"; \
    } \
    var = type::_narrow(tempObj);

Network::Network(int port, const char* networkInterface, const char* algorithm) : sendThreadRunning(true),
    networkBroken(false) {

    pthread_mutex_init(&bindMutex, NULL);
    pthread_mutex_init(&queueMutex, NULL);
    rightIface = new RightNeighbourIface(this);
    leftIface = new LeftNeighbourIface(this);

    // obtain IP address of desired network interface
    ifaddrs* firstAddr;
    ifaddrs* currentAddr;
    char myIPAddr[NI_MAXHOST];

    if(getifaddrs(&firstAddr) == -1) {
        throw "Cannot obatin interface addresses!!!";
    }

    for(currentAddr = firstAddr; currentAddr != NULL; currentAddr = currentAddr->ifa_next) {

        if(currentAddr->ifa_addr == NULL) {
            continue;
        }
        int result = getnameinfo(currentAddr->ifa_addr, sizeof(sockaddr_in), myIPAddr, NI_MAXHOST, NULL, 0,
                                 NI_NUMERICHOST);

        if((strcmp(currentAddr->ifa_name, networkInterface) == 0) && (currentAddr->ifa_addr->sa_family == AF_INET)) {
            if(result != 0) {
                throw "Cannot obatin address of desired network interface!!!";
            }
            break;
        }
    }
    freeifaddrs(firstAddr);

    stringstream thisAddrStr;
    thisAddrStr << "inet:" << myIPAddr << ":" << port;
    myID.identifier = thisAddrStr.str().c_str();
    myID.algorithm = algorithm;

    cout << "Node (address: " << myID.identifier << ") - initializing" << endl;
    reportNodeID = myID;

    int argcORB = 3;
    char** argvORB = new char*[argcORB];

    argvORB[0] = strdup("disco-plat");
    argvORB[1] = strdup("-ORBIIOPAddr");
    argvORB[2] = strdup(myID.identifier);

    try {
        // ORB initialization
        orb = ORB_init(argcORB, argvORB, "mico-local-orb");
        Object_var obj = orb->resolve_initial_references("RootPOA");
        poa = POA::_narrow(obj);

        // creating left and right implementation of boundary interfaces
        RightNeighbourImpl* rightObject = new RightNeighbourImpl();
        poa->activate_object(rightObject);
        rightObject->_this();

        LeftNeighbourImpl* leftObject = new LeftNeighbourImpl();
        poa->activate_object(leftObject);
        leftObject->_this();

        poa->the_POAManager()->activate();
    } catch(Exception& ex) {
        stringstream exStr;
        exStr << "Cannot initialize ORB deamon! The port " << port
              << " is probably bound to another deamon or application!";
        throw exStr.str().c_str();
    }

    cout << "Node (address: " << myID.identifier << ") - initialized" << endl;
}

void Network::start(const char* remoteAddr) {

    pthread_mutex_lock(&bindMutex);
    try {
        // starting recv thread
        if(pthread_create(&recvThread, NULL, &Network::recvThreadMain, this)) {
            throw "Cannot create recieving thread!!!";
        }

        Object_var tempObj;

        if(remoteAddr == NULL) {
            // first node case
            createSingleNodeNetwork();
        } else {
            // non-first node case
            rightID.identifier = remoteAddr;
            BIND_AND_ASSIGN("IDL:disco_plat/LeftNeighbour:1.0", remoteAddr, rightRemoteObject, LeftNeighbour);

            nodeID* leftPtr = &leftID;
            try {
                rightRemoteObject->ConnectAsLeftNode(myID, leftPtr);
            } catch(ConnectionError& ex) {
                stringstream exStr;
                exStr << "Connection refused - reason: " << (const char*)ex.message;
                throw exStr.str().c_str();
            }

            leftID = *leftPtr;
            myID.algorithm = rightID.algorithm = leftID.algorithm;
            BIND_AND_ASSIGN("IDL:disco_plat/RightNeighbour:1.0", (const char*)leftID.identifier, leftRemoteObject,
                            RightNeighbour);

            leftRemoteObject->BuildNetAndRequestData(myID);
        }

        // starting send thread
        if(pthread_create(&sendThread, NULL, &Network::sendThreadMain, this)) {
            throw "Cannot create sending thread!!!";
        }

    } catch(...) {
        pthread_mutex_unlock(&bindMutex);
        throw;
    }
    pthread_mutex_unlock(&bindMutex);
}

void Network::createSingleNodeNetwork() {
    Object_var tempObj;

    rightID = myID;
    BIND_AND_ASSIGN("IDL:disco_plat/LeftNeighbour:1.0", myID.identifier, rightRemoteObject, LeftNeighbour);

    leftID = myID;
    BIND_AND_ASSIGN("IDL:disco_plat/RightNeighbour:1.0", myID.identifier, leftRemoteObject, RightNeighbour);
}

void Network::createSingleNodeNetworkWithMutex() {
    pthread_mutex_lock(&bindMutex);
    try {
        createSingleNodeNetwork();
    } catch(...) {
        pthread_mutex_unlock(&bindMutex);
        throw;
    }
    pthread_mutex_unlock(&bindMutex);
}

Network::~Network() {
    cout << "Node (address: " << myID.identifier << ") - closing network module" << endl;

    sendThreadRunning = false;
    orb->shutdown(TRUE);

    pthread_join(sendThread, NULL);
    pthread_join(recvThread, NULL);

    pthread_mutex_destroy(&queueMutex);
    pthread_mutex_destroy(&bindMutex);

    cout << "Node (address: " << myID.identifier << ") - network module closed" << endl;
}

void Network::enqueItem(QueueItem* item) {
    pthread_mutex_lock(&queueMutex);
    sendQueue.push_back(item);
    pthread_mutex_unlock(&queueMutex);
}


void* Network::recvThreadMain(void* ptr) {

    Network* instance = (Network*)ptr;
    cout << "Node (address: " << instance->myID.identifier << ") - recv thread started" << endl;

    while(true) {
        try {
            instance->orb->run();
            break;
        } catch(SystemException& ex) {
            cerr << "recv thread - Caught CORBA::SystemException." << endl;
            ex._print(cerr);
            cerr << endl;
        } catch(Exception& ex) {
            cerr << "recv thread - Caught CORBA::Exception." << endl;
            ex._print(cerr);
            cerr << endl;
        } catch(...) {
            cerr << "recv thread - Caught unknown exception." << endl;
        }
    }
    return NULL;
}


void* Network::sendThreadMain(void* ptr) {

    Network* instance = (Network*)ptr;
    bool queueIsEmpty;
    QueueItem* current;

    cout << "Node (address: " << instance->myID.identifier << ") - send thread started" << endl;

    while(instance->sendThreadRunning) {
        usleep(100000);

        if(instance->networkBroken) {
            continue;
        }

        pthread_mutex_lock(&instance->queueMutex);
        queueIsEmpty = instance->sendQueue.empty();
        pthread_mutex_unlock(&instance->queueMutex);

        while(!queueIsEmpty) {

            pthread_mutex_lock(&instance->queueMutex);
            current = instance->sendQueue.front();
            instance->sendQueue.pop_front();
            pthread_mutex_unlock(&instance->queueMutex);

            pthread_mutex_lock(&instance->bindMutex);
            try {
                current->sendMe(make_pair(instance->rightRemoteObject, instance->leftRemoteObject));
            } catch(LeftNeighbourCommFailure&) {
                cerr << "send thread - Left neighbour disconnected" << endl;
                pthread_mutex_unlock(&instance->bindMutex);
                instance->reportDeadLeftNode();
                goto mutexJump;
            } catch(RightNeighbourCommFailure&) {
                cerr << "send thread - Right neighbour disconnected" << endl;
                pthread_mutex_unlock(&instance->bindMutex);
                instance->reportDeadRightNode();
                goto mutexJump;
            } catch(SystemException& ex) {
                cerr << "send thread - Caught CORBA::SystemException." << endl;
                ex._print(cerr);
                cerr << endl;
            } catch(Exception& ex) {
                cerr << "send thread - Caught CORBA::Exception." << endl;
                ex._print(cerr);
                cerr << endl;
            } catch(const char* ex) {
                cerr << "send thread - Caught string exception." << endl;
                cerr << ex << endl;
            } catch(...) {
                cerr << "send thread - Caught unknown exception." << endl;
            }
            pthread_mutex_unlock(&instance->bindMutex);
mutexJump:
            if(instance->networkBroken) {
                pthread_mutex_lock(&instance->queueMutex);
                instance->sendQueue.push_front(current);
                pthread_mutex_unlock(&instance->queueMutex);
                break;
            }

            delete current;

            pthread_mutex_lock(&instance->queueMutex);
            queueIsEmpty = instance->sendQueue.empty();
            pthread_mutex_unlock(&instance->queueMutex);
        }
    }

    return NULL;
}

RightNeighbourIface& Network::getMyRightInterface() {
    return *rightIface;
}

LeftNeighbourIface& Network::getMyLeftInterface() {
    return *leftIface;
}

void Network::changeRightNeighbour(const nodeID& newID) {
    pthread_mutex_lock(&bindMutex);
    try {
        Object_var tempObj;
        rightID = newID;
        BIND_AND_ASSIGN("IDL:disco_plat/LeftNeighbour:1.0", (const char*)newID.identifier, rightRemoteObject,
                        LeftNeighbour);
    } catch(...) {
        pthread_mutex_unlock(&bindMutex);
        throw;
    }
    pthread_mutex_unlock(&bindMutex);
}

void Network::changeLeftNeighbour(const nodeID& newID) {
    pthread_mutex_lock(&bindMutex);
    try {
        Object_var tempObj;
        leftID = newID;
        BIND_AND_ASSIGN("IDL:disco_plat/RightNeighbour:1.0", (const char*)newID.identifier, leftRemoteObject,
                        RightNeighbour);
    } catch(...) {
        pthread_mutex_unlock(&bindMutex);
        throw;
    }
    pthread_mutex_unlock(&bindMutex);
}

void Network::reportDeadLeftNode() {

    pthread_mutex_lock(&bindMutex);
    try {

        Object_var tempObj;
        leftID = reportNodeID;
        BIND_AND_ASSIGN("IDL:disco_plat/RightNeighbour:1.0", (const char*)reportNodeID.identifier, leftRemoteObject,
                        RightNeighbour);

        leftRemoteObject->RebuildNetwork(myID);

        set<string> liveNodesSet;
        for(unsigned i = 0; i < liveNodes.length(); ++i) {
            liveNodesSet.insert((const char*)liveNodes[i].identifier);
        }
        repo->setLiveNodes(liveNodesSet);

        set<unsigned> compIDSet;
        for(unsigned i = 0; i < liveCompIDs.length(); ++i) {
            compIDSet.insert(liveCompIDs[i]);
        }
        repo->setSurvivingComputations(compIDSet);

        blob data;
        data.sourceNode = myID;
        data.messageType = NETWORK_REBUILT;
        data.nodeIDSequence = liveNodes;
        data.longDataSequence = liveCompIDs;
        getMyRightInterface().Boomerang(data);

    } catch(...) {
        // EPIC fail!!!
        sendBoomerangAndAbort(rightRemoteObject);
    }
    pthread_mutex_unlock(&bindMutex);

}

void Network::reportDeadRightNode() {
    if(!networkBroken) {
        try {
            SequenceTmpl<nodeID, MICO_TID_DEF> newNodeSequence;
            SequenceTmpl<Long, MICO_TID_DEF> newCompSequence;

            newNodeSequence.length(1);
            newCompSequence.length(1);

            newNodeSequence[0] = myID;
            newCompSequence[0] = currentSyncModule->getComputationID();

            leftRemoteObject->NodeDied(getMyID(), newNodeSequence, newCompSequence);
            networkBroken = true;
        } catch(COMM_FAILURE&) {
            // I am alone...
            set<string> liveNodesSet;
            liveNodesSet.insert((const char*)myID.identifier);
            repo->setLiveNodes(liveNodesSet);

            set<unsigned> compIDSet;
            compIDSet.insert(currentSyncModule->getComputationID());
            repo->setSurvivingComputations(compIDSet);

            createSingleNodeNetworkWithMutex();
        }
    }
}
