#ifndef NEIGHBOURIMPL_H
#define NEIGHBOURIMPL_H

#include "../build/Interface.h"

class LeftNeighbourImpl : virtual public POA_disco_plat::LeftNeighbour
{

public:

    virtual ::disco_plat::nodeID* ConnectAsLeftNode(const ::disco_plat::nodeID& newNodeID);
    virtual void NeigbourDied(const ::disco_plat::nodeID& reportingNodeID);
    virtual void UpdateRightNode(const ::disco_plat::nodeID& newNodeID);
    virtual void UpdateLeftNode(const ::disco_plat::nodeID& newNodeID);
    virtual void Boomerang(const ::disco_plat::blob& data);

};

class RightNeighbourImpl : virtual public POA_disco_plat::RightNeighbour
{

public:

    virtual ::disco_plat::nodeID* ConnectAsLeftNode(const ::disco_plat::nodeID& newNodeID);
    virtual void NeigbourDied(const ::disco_plat::nodeID& reportingNodeID);
    virtual void UpdateRightNode(const ::disco_plat::nodeID& newNodeID);
    virtual void UpdateLeftNode(const ::disco_plat::nodeID& newNodeID);
    virtual void Boomerang(const ::disco_plat::blob& data);

};

#endif // NEIGHBOURIMPL_H
