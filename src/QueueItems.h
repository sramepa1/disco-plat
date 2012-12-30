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
};


/*****************************************************************/
//  LeftNeighbour

class Left_NodeDied : public QueueItem {
    const ::disco_plat::nodeID reportingNodeID;
    const ::disco_plat::nodeID& deadNodeID;
public:
    Left_NodeDied(const ::disco_plat::nodeID& reportingNodeID, const ::disco_plat::nodeID& deadNodeID)
        : reportingNodeID(reportingNodeID), deadNodeID(deadNodeID) {}
    void sendMe(NeighbourPair neighbours) {
        try {
            neighbours.second->NodeDied(reportingNodeID, deadNodeID);
        } catch (CORBA::COMM_FAILURE&) {
            throw LeftNeighbourCommFailure();
        }
    }
};

class Left_RebuildNetwork : public QueueItem {
    const ::disco_plat::nodeID newNeighbourID;
public:
    Left_RebuildNetwork(const ::disco_plat::nodeID& newNeighbourID) : newNeighbourID(newNeighbourID) {}
    void sendMe(NeighbourPair neighbours) {
        neighbours.second->RebuildNetwork(newNeighbourID);
    }
};


/*****************************************************************/
//  RightNeighbour

class Right_Boomerang : public QueueItem {
    const ::disco_plat::blob data;
public:
    Right_Boomerang(const ::disco_plat::blob& data) : data(data) {}
    void sendMe(NeighbourPair neighbours) {
        try {
            neighbours.first->Boomerang(data);
        } catch (CORBA::COMM_FAILURE&) {
            throw RightNeighbourCommFailure();
        }
    }
};

#endif // QUEUEITEMS_H
