#ifndef NEIGHBOURIFACE_H
#define NEIGHBOURIFACE_H

#include "Network.h"

class LeftNeighbourIface
{

    Network* parent;
    LeftNeighbourIface(Network* parent) : parent(parent) {}

public:
    //virtual ::disco_plat::nodeID* ConnectAsLeftNode(const ::disco_plat::nodeID& newNodeID) {}
    virtual void NeigbourDied(const ::disco_plat::nodeID& reportingNodeID) {}
    virtual void UpdateRightNode(const ::disco_plat::nodeID& newNodeID) {}
    virtual void UpdateLeftNode(const ::disco_plat::nodeID& newNodeID) {}
    virtual void Boomerang(const ::disco_plat::blob& data) {}
};

class RightNeighbourIface
{

    Network* parent;
    RightNeighbourIface(Network* parent) : parent(parent) {}

public:
    //virtual ::disco_plat::nodeID* ConnectAsLeftNode(const ::disco_plat::nodeID& newNodeID) {}
    virtual void NeigbourDied(const ::disco_plat::nodeID& reportingNodeID) {}
    virtual void UpdateRightNode(const ::disco_plat::nodeID& newNodeID) {}
    virtual void UpdateLeftNode(const ::disco_plat::nodeID& newNodeID) {}
    virtual void Boomerang(const ::disco_plat::blob& data) {}
};


#endif // NEIGHBOURIFACE_H
