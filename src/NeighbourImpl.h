#ifndef NEIGHBOURIMPL_H
#define NEIGHBOURIMPL_H

#include "../build/Interface.h"

class LeftNeighbourImpl : virtual public POA_disco_plat::LeftNeighbour {

public:
    virtual void ConnectAsLeftNode(const ::disco_plat::nodeID& newNodeID, ::disco_plat::nodeID_out oldLeftNodeID);
    virtual void Boomerang(const ::disco_plat::blob& data);
    virtual void AbortingBoomerang();
};

class RightNeighbourImpl : virtual public POA_disco_plat::RightNeighbour {

public:
    virtual void BuildNetAndRequestData(const ::disco_plat::nodeID& newNeighbourID);
    virtual void NodeDied(const ::disco_plat::nodeID& reportingNodeID,
                          SequenceTmpl< ::disco_plat::nodeID, MICO_TID_DEF> liveNodes);
    virtual void RebuildNetwork(const ::disco_plat::nodeID& newNeighbourID);
};

#endif // NEIGHBOURIMPL_H
