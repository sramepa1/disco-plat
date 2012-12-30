#ifndef NEIGHBOURIMPL_H
#define NEIGHBOURIMPL_H

#include "../build/Interface.h"

class LeftNeighbourImpl : virtual public POA_disco_plat::LeftNeighbour {

public:
    virtual void ConnectAsLeftNode(const ::disco_plat::nodeID& newNodeID, ::disco_plat::nodeID_out oldLeftNodeID);
    virtual void NeigbourDied(const ::disco_plat::nodeID& reportingNodeID, const ::disco_plat::nodeID& deadNodeID);
    virtual void UpdateRightNode(const ::disco_plat::nodeID& newNodeID);
    virtual void UpdateLeftNode(const ::disco_plat::nodeID& newNodeID);
    virtual void Boomerang(const ::disco_plat::blob& data);
};

class RightNeighbourImpl : virtual public POA_disco_plat::RightNeighbour {

public:
    virtual void BuildNetAndRequestData(const ::disco_plat::nodeID& newNeighbourID);
    //virtual void RequestComputationalData(const ::disco_plat::nodeID& destinationID);
    virtual void NeighbourDied(const ::disco_plat::nodeID& reportingNodeID, const ::disco_plat::nodeID& deadNodeID);
    virtual void RebuildNetwork(const ::disco_plat::nodeID& newNeighbourID);
//    virtual void UpdateRightNode(const ::disco_plat::nodeID& newNodeID);
//    virtual void UpdateLeftNode(const ::disco_plat::nodeID& newNodeID);
};

#endif // NEIGHBOURIMPL_H
