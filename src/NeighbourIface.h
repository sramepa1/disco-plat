#ifndef NEIGHBOURIFACE_H
#define NEIGHBOURIFACE_H

#include "Network.h"
#include "QueueItems.h"

class LeftNeighbourIface {

    Network* parent;
    LeftNeighbourIface(Network* parent) : parent(parent) {}
    friend class Network;

public:
    void NeighbourDied(const ::disco_plat::nodeID& reportingNodeID, const ::disco_plat::nodeID& deadNodeID) {
        parent->enqueItem(new Left_NodeDied(reportingNodeID, deadNodeID));
    }
};

class RightNeighbourIface {

    Network* parent;
    RightNeighbourIface(Network* parent) : parent(parent) {}
    friend class Network;

public:
    void Boomerang(const ::disco_plat::blob& data) {
        parent->enqueItem(new Right_Boomerang(data));
    }
    void AbortingBoomerang() {
        parent->enqueItem(new Right_AbortingBoomerang());
    }
};

#endif // NEIGHBOURIFACE_H
