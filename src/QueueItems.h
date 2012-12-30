#ifndef QUEUEITEMS_H
#define QUEUEITEMS_H

#include <utility>

#include "../build/Interface.h"

typedef std::pair<disco_plat::LeftNeighbour_var, disco_plat::RightNeighbour_var> NeighbourPair;

class QueueItem {
public:
    virtual ~QueueItem() {}
    virtual void sendMe(NeighbourPair neighbours) = 0;
};


/*****************************************************************/
//  LeftNeighbour

class Left_RequestComputationalData : public QueueItem {
    const ::disco_plat::nodeID& destinationID;
public:
    Left_RequestComputationalData(const ::disco_plat::nodeID& destinationID) : destinationID(destinationID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.second->RequestComputationalData(destinationID);
    }
};

class Left_NeigbourDied : public QueueItem {
    const ::disco_plat::nodeID reportingNodeID;
    const ::disco_plat::nodeID& deadNodeID;
public:
    Left_NeigbourDied(const ::disco_plat::nodeID& reportingNodeID, const ::disco_plat::nodeID& deadNodeID)
        : reportingNodeID(reportingNodeID), deadNodeID(deadNodeID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.second->NeigbourDied(reportingNodeID, deadNodeID);
    }
};

class Left_UpdateRightNode : public QueueItem {
    const ::disco_plat::nodeID newNodeID;
public:
    Left_UpdateRightNode(const ::disco_plat::nodeID& newNodeID) : newNodeID(newNodeID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.second->UpdateRightNode(newNodeID);
    }
};

class Left_UpdateLeftNode : public QueueItem {
    const ::disco_plat::nodeID newNodeID;
public:
    Left_UpdateLeftNode(const ::disco_plat::nodeID& newNodeID) : newNodeID(newNodeID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.second->UpdateLeftNode(newNodeID);
    }
};


/*****************************************************************/
//  RightNeighbour

class Right_NeigbourDied : public QueueItem {
    const ::disco_plat::nodeID reportingNodeID;
    const ::disco_plat::nodeID& deadNodeID;
public:
    Right_NeigbourDied(const ::disco_plat::nodeID& reportingNodeID, const ::disco_plat::nodeID& deadNodeID)
        : reportingNodeID(reportingNodeID), deadNodeID(deadNodeID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.first->NeigbourDied(reportingNodeID, deadNodeID);
    }
};

class Right_UpdateRightNode : public QueueItem {
    const ::disco_plat::nodeID newNodeID;
public:
    Right_UpdateRightNode(const ::disco_plat::nodeID& newNodeID) : newNodeID(newNodeID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.first->UpdateRightNode(newNodeID);
    }
};

class Right_UpdateLeftNode : public QueueItem {
    const ::disco_plat::nodeID newNodeID;
public:
    Right_UpdateLeftNode(const ::disco_plat::nodeID& newNodeID) : newNodeID(newNodeID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.first->UpdateLeftNode(newNodeID);
    }
};

class Right_Boomerang : public QueueItem {
    const ::disco_plat::blob data;
public:
    Right_Boomerang(const ::disco_plat::blob& data) : data(data) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.first->Boomerang(data);
    }
};

#endif // QUEUEITEMS_H
