#ifndef QUEUEITEMS_H
#define QUEUEITEMS_H

#include <utility>
#include <cstdlib>
#include <iostream>

#include "../build/Interface.h"

typedef std::pair<disco_plat::LeftNeighbour_var, disco_plat::RightNeighbour_var> NeighbourPair;

class Network;

class LeftNeighbourCommFailure {};
class RightNeighbourCommFailure {};


// Helper function
inline void sendBoomerangAndAbort(disco_plat::LeftNeighbour_var remoteObject) {
    try {
        remoteObject->AbortingBoomerang();
    } catch (CORBA::COMM_FAILURE&) {
        // I am the last one
    }
    std::cerr << "Two or more nodes have been diconnected from the network!" << std::endl
              << "Cannot continue, aborting process!" << std::endl;
    abort();   // Goodbye, cruel world!!!
}


class QueueItem {
public:
    virtual ~QueueItem() {}
    virtual void sendMe(NeighbourPair neighbours) = 0;
};


/*****************************************************************/
//  LeftNeighbour

class Left_NodeDied : public QueueItem {
    const ::disco_plat::nodeID& reportingNodeID;
    const SequenceTmpl< ::disco_plat::nodeID, MICO_TID_DEF> liveNodes;
    const SequenceTmpl<CORBA::Long,MICO_TID_DEF> compIDs;
public:
    Left_NodeDied(const ::disco_plat::nodeID& reportingNodeID,
                  const SequenceTmpl< ::disco_plat::nodeID, MICO_TID_DEF>& liveNodes,
                  const SequenceTmpl<CORBA::Long,MICO_TID_DEF>& compIDs)
        : reportingNodeID(reportingNodeID), liveNodes(liveNodes), compIDs(compIDs) {}

    void sendMe(NeighbourPair neighbours) {
        try {
            neighbours.second->NodeDied(reportingNodeID, liveNodes, compIDs);
        } catch (CORBA::COMM_FAILURE&) {
            throw LeftNeighbourCommFailure();
        }
    }
};

/*****************************************************************/
//  RightNeighbour

class Right_Boomerang : public QueueItem {
    friend class Network;
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

class Right_AbortingBoomerang : public QueueItem {
public:
    void sendMe(NeighbourPair neighbours) {
        sendBoomerangAndAbort(neighbours.first);
    }
};

#endif // QUEUEITEMS_H
