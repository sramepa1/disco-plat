#ifndef NEIGHBOURIFACE_H
#define NEIGHBOURIFACE_H

#include "Network.h"
#include "QueueItems.h"

class LeftNeighbourIface {

    Network* parent;
    LeftNeighbourIface(Network* parent) : parent(parent) {}
    friend class Network;

public:
    void RequestComputationalData(const ::disco_plat::nodeID& destinationID) {
        parent->enqueItem(new Left_RequestComputationalData(destinationID));
    }
    void NeigbourDied(const ::disco_plat::nodeID& reportingNodeID, const ::disco_plat::nodeID& deadNodeID) {
        parent->enqueItem(new Left_NeigbourDied(reportingNodeID, deadNodeID));
    }
    void UpdateRightNode(const ::disco_plat::nodeID& newNodeID) {
        parent->enqueItem(new Left_UpdateRightNode(newNodeID));
    }
    void UpdateLeftNode(const ::disco_plat::nodeID& newNodeID) {
        parent->enqueItem(new Left_UpdateLeftNode(newNodeID));
    }
};

class RightNeighbourIface {

    Network* parent;
    RightNeighbourIface(Network* parent) : parent(parent) {}
    friend class Network;

public:
    void NeigbourDied(const ::disco_plat::nodeID& reportingNodeID, const ::disco_plat::nodeID& deadNodeID) {
        parent->enqueItem(new Right_NeigbourDied(reportingNodeID, deadNodeID));
    }
    void UpdateRightNode(const ::disco_plat::nodeID& newNodeID) {
        parent->enqueItem(new Right_UpdateRightNode(newNodeID));
    }
    void UpdateLeftNode(const ::disco_plat::nodeID& newNodeID) {
        parent->enqueItem(new Right_UpdateLeftNode(newNodeID));
    }
    void Boomerang(const ::disco_plat::blob& data) {
        parent->enqueItem(new Right_Boomerang(data));
    }
};

#endif // NEIGHBOURIFACE_H
