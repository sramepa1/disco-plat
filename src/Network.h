#ifndef NETWORK_H
#define NETWORK_H

#include <deque>
#include <string>

#include <pthread.h>
#include <CORBA.h>

#include "QueueItems.h"

class LeftNeighbourIface;
class RightNeighbourIface;

class Network {

    CORBA::ORB_var orb;
    PortableServer::POA_var poa;

    std::string myAddr;

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
    RightNeighbourIface* rightIface;

    disco_plat::nodeID leftID;
    disco_plat::RightNeighbour_var leftRemoteObject;
    LeftNeighbourIface* leftIface;

public:
    Network(int port, const char* remoteAddr);
    ~Network();

    void enqueItem(QueueItem* item);

    RightNeighbourIface& getMyRightInterface();
    LeftNeighbourIface& getMyLeftInterface();

    void changeRightNeighbour(disco_plat::nodeID newID);
    void changeLeftNeighbour(disco_plat::nodeID newID);

private:
    Network(const Network&) {}
    void operator= (const Network&) {}
};

#endif // NETWORK_H
