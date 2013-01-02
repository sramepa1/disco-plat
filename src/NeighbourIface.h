#ifndef NEIGHBOURIFACE_H
#define NEIGHBOURIFACE_H

#include "Repository.h"
#include "globals.h"
#include "Network.h"
#include "QueueItems.h"

class LeftNeighbourIface {

    Network* parent;
    LeftNeighbourIface(Network* parent) : parent(parent) {}
    friend class Network;

public:
    void NeighbourDied(const ::disco_plat::nodeID& reportingNodeID,
                       const SequenceTmpl< ::disco_plat::nodeID, MICO_TID_DEF>& liveNodes,
                       const SequenceTmpl<CORBA::Long,MICO_TID_DEF>& compIDs) {
        parent->enqueItem(new Left_NodeDied(reportingNodeID, liveNodes, compIDs));
    }
};

class RightNeighbourIface {

    Network* parent;
    RightNeighbourIface(Network* parent) : parent(parent) {}
    friend class Network;

public:
    void Boomerang(::disco_plat::blob& data) {
        data.timestamp = repo->timeIncrement();
        parent->enqueItem(new Right_Boomerang(data));
    }
    void AbortingBoomerang() {
        parent->enqueItem(new Right_AbortingBoomerang());
    }
};

#endif // NEIGHBOURIFACE_H
