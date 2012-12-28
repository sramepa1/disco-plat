#include "Network.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cstring>

#include <sys/utsname.h>
#include <unistd.h>

#include "../build/Interface.h"
#include "NeighbourIface.h"
#include "NeighbourImpl.h"

using namespace std;
using namespace disco_plat;

#define BIND_AND_ASSIGN(objectIDL, addr, var, type) \
    tempObj = orb->bind(objectIDL, addr); \
    if (CORBA::is_nil(tempObj)) { \
        cerr << "Node (port " << myAddr << ") - cannot bind to " << addr << endl; \
        throw "Cannot bind!"; \
    } \
    var = type::_narrow(tempObj);

Network::Network(int port, const char* remoteAddr) : sendThreadRunning(true) {

    pthread_mutex_init(&queueMutex, NULL);
    rightIface = new RightNeighbourIface(this);
    leftIface = new LeftNeighbourIface(this);

    utsname myUname;
    uname(&myUname);

    stringstream thisAddrStr;
    thisAddrStr << "inet:" << myUname.nodename << ":" << port;
    myAddr = thisAddrStr.str();

    cout << "Node (address: " << myAddr << ") - initializing" << endl;

    int argcORB = 3;
    char** argvORB = new char*[argcORB];

    argvORB[0] = new char[strlen("disco-plat") + 1];
    memcpy(argvORB[0], "disco-plat", strlen("disco-plat") + 1);

    argvORB[1] = new char[strlen("-ORBIIOPAddr") + 1];
    memcpy(argvORB[1], "-ORBIIOPAddr", strlen("-ORBIIOPAddr") + 1);

    argvORB[2] = new char[myAddr.size() + 1];
    memcpy(argvORB[2], myAddr.c_str(), myAddr.size() + 1);

    // initialization
    orb = CORBA::ORB_init(argcORB, argvORB, "mico-local-orb");
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    poa = PortableServer::POA::_narrow(obj);

    // creating left and right implementation of boundary interfaces
    RightNeighbourImpl* rightObject = new RightNeighbourImpl();
    poa->activate_object(rightObject);
    rightObject->_this();

    LeftNeighbourImpl* leftObject = new LeftNeighbourImpl();
    poa->activate_object(leftObject);
    leftObject->_this();

    // starting recv thread
    if(pthread_create(&recvThread, NULL, &Network::recvThreadMain, this)) {
        throw "Cannot create recieving thread!!!";
    }

    CORBA::Object_var tempObj;

    if(remoteAddr == NULL) {
        // first node case
        rightID.identifier = myAddr.c_str();
        BIND_AND_ASSIGN("IDL:disco_plat/LeftNeighbour:1.0", myAddr.c_str(), rightRemoteObject, LeftNeighbour);

        leftID.identifier = myAddr.c_str();
        BIND_AND_ASSIGN("IDL:disco_plat/RightNeighbour:1.0", myAddr.c_str(), leftRemoteObject, RightNeighbour);

    } else {
        // non-first node case
        rightID.identifier = remoteAddr;
        BIND_AND_ASSIGN("IDL:disco_plat/LeftNeighbour:1.0", remoteAddr, rightRemoteObject, LeftNeighbour);

        nodeID myID;
        myID.identifier = myAddr.c_str();
        leftID = *leftRemoteObject->ConnectAsLeftNode(myID);
        BIND_AND_ASSIGN("IDL:disco_plat/RightNeighbour:1.0", (char*)leftID.identifier, leftRemoteObject, RightNeighbour);

        getMyLeftInterface().UpdateRightNode(myID);
    }

    // starting send thread
    if(pthread_create(&sendThread, NULL, &Network::sendThreadMain, this)) {
        throw "Cannot create sending thread!!!";
    }
}

Network::~Network() {
    // TODO: join threads
    sendThreadRunning = false;
    orb->shutdown(TRUE);
}

void Network::enqueItem(QueueItem* item) {
    pthread_mutex_lock(&queueMutex);
    sendQueue.push_back(item);
    pthread_mutex_unlock(&queueMutex);
}


void* Network::recvThreadMain(void* ptr) {

    Network* instance = (Network*)ptr;
    cout << "Node (address: " << instance->myAddr << ") - recv thread started" << endl;
    instance->poa->the_POAManager()->activate();

    while(true) {
        try {
            instance->orb->run();
            break;
        } catch(CORBA::SystemException& ex) {       // TODO: better exception handling
            cerr << "Caught CORBA::SystemException." << endl;
            ex._print(cerr);
            cerr << endl;
        } catch(CORBA::Exception& ex) {
            cerr << "Caught CORBA::Exception." << endl;
            ex._print(cerr);
            cerr << endl;
        } catch(...) {
            cerr << "Caught unknown exception." << endl;
        }
    }
    return NULL;
}


void* Network::sendThreadMain(void* ptr) {

    Network* instance = (Network*)ptr;
    bool queueIsEmpty;
    QueueItem* current;

    cout << "Node (address: " << instance->myAddr << ") - send thread started" << endl;

    while(instance->sendThreadRunning) {
        usleep(100000);

        pthread_mutex_lock(&instance->queueMutex);
        queueIsEmpty = instance->sendQueue.empty();
        pthread_mutex_unlock(&instance->queueMutex);

        while(!queueIsEmpty) {

            pthread_mutex_lock(&instance->queueMutex);
            current = instance->sendQueue.front();
            instance->sendQueue.pop_front();
            pthread_mutex_unlock(&instance->queueMutex);

            try {
                current->sendMe(make_pair(instance->rightRemoteObject, instance->leftRemoteObject));
            } catch(CORBA::SystemException& ex) {       // TODO: better exception handling
                cerr << "Caught CORBA::SystemException." << endl;
                ex._print(cerr);
                cerr << endl;
            } catch(CORBA::Exception& ex) {
                cerr << "Caught CORBA::Exception." << endl;
                ex._print(cerr);
                cerr << endl;
            } catch(...) {
                cerr << "Caught unknown exception." << endl;
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

// TODO: mutex and try-catch - it is needed?
void Network::changeRightNeighbour(nodeID newID) {
    CORBA::Object_var tempObj;
    rightID = newID;
    BIND_AND_ASSIGN("IDL:disco_plat/LeftNeighbour:1.0", (char*)newID.identifier, rightRemoteObject, LeftNeighbour);
}

void Network::changeLeftNeighbour(nodeID newID) {
    CORBA::Object_var tempObj;
    leftID = newID;
    BIND_AND_ASSIGN("IDL:disco_plat/RightNeighbour:1.0", (char*)newID.identifier, leftRemoteObject, RightNeighbour);
}
