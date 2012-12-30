#ifndef NETWORK_H
#define NETWORK_H

#include <deque>
#include <string>

#include <pthread.h>
#include <CORBA.h>

#include "QueueItems.h"

class LeftNeighbourIface;
class RightNeighbourIface;

enum NodeState {
    OK, DEAD, MAYBE_DEAD
};

class Network {

    CORBA::ORB_var orb;
    PortableServer::POA_var poa;

    disco_plat::nodeID myID;

    std::deque<QueueItem*> sendQueue;
    pthread_mutex_t queueMutex;

    pthread_t recvThread;
    pthread_t sendThread;

    bool sendThreadRunning;
    pthread_mutex_t bindMutex;

    static void* recvThreadMain(void* ptr);
    static void* sendThreadMain(void* ptr);

    // left and right neighbour
    disco_plat::nodeID rightID;
    disco_plat::LeftNeighbour_var rightRemoteObject;
    RightNeighbourIface* rightIface;
    NodeState rightNeighbourState;

    disco_plat::nodeID leftID;
    disco_plat::RightNeighbour_var leftRemoteObject;
    LeftNeighbourIface* leftIface;
    NodeState leftNeighbourState;

    void createSingleNodeNetwork();
    void createSingleNodeNetworkWithMutex();

    void reportDeadLeftNode();
    void reportDeadRightNode();

    // TODO: set initial value
    disco_plat::nodeID reportNodeID;
    bool networkBroken;

public:
    Network(int port, const char* networkInterface, const char* algorithm);
    ~Network();

    void start(const char* remoteAddr);
    void enqueItem(QueueItem* item);

    RightNeighbourIface& getMyRightInterface();
    LeftNeighbourIface& getMyLeftInterface();

    const disco_plat::nodeID& getMyID() { return myID; }
    const disco_plat::nodeID& getRightID() { return rightID; }
    const disco_plat::nodeID& getLeftID() { return leftID; }

    void changeRightNeighbour(const disco_plat::nodeID& newID);
    void changeLeftNeighbour(const disco_plat::nodeID& newID);

    NodeState getRightNeighbourState() { return rightNeighbourState; }
    NodeState getLeftNeighbourState() { return leftNeighbourState; }
    void setRightNeighbourState(NodeState newState) { rightNeighbourState = newState; }
    void setLeftNeighbourState(NodeState newState) { leftNeighbourState = newState; }

    void setReportNodeID(disco_plat::nodeID newID) { reportNodeID = newID; }
    void repairNetwork() { networkBroken = false; }     // It's so simple... :-)

private:
    Network(const Network&) {}
    void operator=(const Network&) {}
};

#endif // NETWORK_H
