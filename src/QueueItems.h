#ifndef QUEUEITEMS_H
#define QUEUEITEMS_H

#include <utility>

#include "../build/Interface.h"

typedef std::pair<disco_plat::LeftNeighbour_var, disco_plat::RightNeighbour_var> NeighbourPair;

class LeftNeighbourCommFailure {};
class RightNeighbourCommFailure {};

class QueueItem {
public:
    virtual ~QueueItem() {}
    virtual void sendMe(NeighbourPair neighbours) = 0;
    virtual void sendMeAndThrow(NeighbourPair neighbours) = 0;
};


/*****************************************************************/
//  LeftNeighbour

class Left_QueueItem : public QueueItem {
public:
    virtual ~Left_QueueItem() {}
    void sendMeAndThrow(NeighbourPair neighbours) {
        try {
            sendMe(neighbours);
        } catch (CORBA::COMM_FAILURE&) {
            throw LeftNeighbourCommFailure();
        }
    }
};

//class Left_RequestComputationalData : public Left_QueueItem {
//    const ::disco_plat::nodeID& destinationID;
//public:
//    Left_RequestComputationalData(const ::disco_plat::nodeID& destinationID) : destinationID(destinationID) {}
//    void sendMe(NeighbourPair neighbours) {
//        neighbours.second->RequestComputationalData(destinationID);
//    }
//};

class Left_NeighbourDied : public Left_QueueItem {
    const ::disco_plat::nodeID reportingNodeID;
    const ::disco_plat::nodeID& deadNodeID;
public:
    Left_NeighbourDied(const ::disco_plat::nodeID& reportingNodeID, const ::disco_plat::nodeID& deadNodeID)
        : reportingNodeID(reportingNodeID), deadNodeID(deadNodeID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.second->NeighbourDied(reportingNodeID, deadNodeID);
    }
};

class Left_RebuildNetwork : public Left_QueueItem {
    const ::disco_plat::nodeID newNeighbourID;
public:
    Left_RebuildNetwork(const ::disco_plat::nodeID& newNeighbourID) : newNeighbourID(newNeighbourID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.second->RebuildNetwork(newNeighbourID);
    }
};

//class Left_UpdateLeftNode : public Left_QueueItem {
//    const ::disco_plat::nodeID newNodeID;
//public:
//    Left_UpdateLeftNode(const ::disco_plat::nodeID& newNodeID) : newNodeID(newNodeID) {}
//    void sendMe(NeighbourPair neighbours) {
//        neighbours.second->UpdateLeftNode(newNodeID);
//    }
//};


/*****************************************************************/
//  RightNeighbour

class Right_QueueItem : public QueueItem {
public:
    virtual ~Right_QueueItem() {}
    void sendMeAndThrow(NeighbourPair neighbours) {
        try {
            sendMe(neighbours);
        } catch (CORBA::COMM_FAILURE&) {
            throw RightNeighbourCommFailure();
        }
    }
};

//class Right_NeigbourDied : public Right_QueueItem {
//    const ::disco_plat::nodeID reportingNodeID;
//    const ::disco_plat::nodeID& deadNodeID;
//public:
//    Right_NeigbourDied(const ::disco_plat::nodeID& reportingNodeID, const ::disco_plat::nodeID& deadNodeID)
//        : reportingNodeID(reportingNodeID), deadNodeID(deadNodeID) {}
//    void sendMe(NeighbourPair neighbours) {
//        neighbours.first->NeigbourDied(reportingNodeID, deadNodeID);
//    }
//};

//class Right_UpdateRightNode : public Right_QueueItem {
//    const ::disco_plat::nodeID newNodeID;
//public:
//    Right_UpdateRightNode(const ::disco_plat::nodeID& newNodeID) : newNodeID(newNodeID) {}
//    void sendMe(NeighbourPair neighbours) {
//        neighbours.first->UpdateRightNode(newNodeID);
//    }
//};

//class Right_UpdateLeftNode : public Right_QueueItem {
//    const ::disco_plat::nodeID newNodeID;
//public:
//    Right_UpdateLeftNode(const ::disco_plat::nodeID& newNodeID) : newNodeID(newNodeID) {}
//    void sendMe(NeighbourPair neighbours) {
//        neighbours.first->UpdateLeftNode(newNodeID);
//    }
//};

class Right_Boomerang : public Right_QueueItem {
    const ::disco_plat::blob data;
public:
    Right_Boomerang(const ::disco_plat::blob& data) : data(data) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.first->Boomerang(data);
    }
};

#endif // QUEUEITEMS_H
