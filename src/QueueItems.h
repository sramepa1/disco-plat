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

class Left_NeigbourDied : public QueueItem {
    const ::disco_plat::nodeID reportingNodeID;
public:
    Left_NeigbourDied(const ::disco_plat::nodeID& reportingNodeID) : reportingNodeID(reportingNodeID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.second->NeigbourDied(reportingNodeID);
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

class Left_Boomerang : public QueueItem {
    const ::disco_plat::blob data;
public:
    Left_Boomerang(const ::disco_plat::blob& data) : data(data) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.second->Boomerang(data);
    }
};


/*****************************************************************/
//  RightNeighbour

class Right_NeigbourDied : public QueueItem {
    const ::disco_plat::nodeID reportingNodeID;
public:
    Right_NeigbourDied(const ::disco_plat::nodeID& reportingNodeID) : reportingNodeID(reportingNodeID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.first->NeigbourDied(reportingNodeID);
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
