#ifndef NETWORK_H
#define NETWORK_H

#include <deque>

#include <pthread.h>
#include <CORBA.h>

#include "QueueItems.h"

class Network
{

    CORBA::ORB_var orb;
    PortableServer::POA_var poa;

    std::deque<QueueItem*> sendQueue;
    pthread_mutex_t queueMutex;

    pthread_t recvThread;
    pthread_t sendThread;
    bool sendThreadRunning;

    static void* recvThreadMain(void* ptr);
    static void* sendThreadMain(void* ptr);

    // left and right neighbour
    disco_plat::nodeID rightID;
    disco_plat::LeftNeighbour_var rightRemoteObject;

    disco_plat::nodeID leftID;
    disco_plat::RightNeighbour_var leftRemoteObject;


public:
    Network(int port, const char* remoteAddr);
    ~Network();
};

#endif // NETWORK_H
