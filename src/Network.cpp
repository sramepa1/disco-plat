#include "Network.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cstring>

#include <sys/utsname.h>
#include <unistd.h>

#include "../build/Interface.h"
#include "NeighbourIface.h"

using namespace std;
using namespace disco_plat;


Network::Network(int port, const char* remoteAddr) : sendThreadRunning(true) {

    rightIface = new RightNeighbourIface(this);
    leftIface = new LeftNeighbourIface(this);

    utsname myUname;
    uname(&myUname);

    stringstream thisAddr;
    thisAddr << "inet:" << myUname.nodename << ":" << port;

    cout << "Node (address: " << thisAddr.str() << ") - initializing" << endl;

    int argcORB = 3;
    char** argvORB = new char*[argcORB];

    argvORB[0] = new char[strlen("disco-plat") + 1];
    memcpy(argvORB[1], "disco-plat", strlen("disco-plat") + 1);

    argvORB[1] = new char[strlen("-ORBIIOPAddr") + 1];
    memcpy(argvORB[1], "-ORBIIOPAddr", strlen("-ORBIIOPAddr") + 1);

    argvORB[2] = new char[thisAddr.str().size() + 1];
    memcpy(argvORB[2], thisAddr.str().c_str(), thisAddr.str().size() + 1);


    // initialization
    orb = CORBA::ORB_init(argcORB, argvORB, "mico-local-orb");
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    poa = PortableServer::POA::_narrow(obj);

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

LeftNeighbourIface &Network::getMyLeftInterface() {
    return *leftIface;
}

