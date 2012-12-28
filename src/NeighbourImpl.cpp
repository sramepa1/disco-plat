#include "NeighbourImpl.h"

#include "Network.h"
#include "globals.h"

#include <iostream>

using namespace std;
using namespace disco_plat;

/*****************************************************************/
//  LeftNeighbour

nodeID* LeftNeighbourImpl::ConnectAsLeftNode(const nodeID& newNodeID) {

    cout << "Recieved message ConnectAsLeftNode from left neighbour" << endl;

    nodeID* oldID = new nodeID(networkModule->getLeftID());
    networkModule->changeLeftNeighbour(newNodeID);
    return oldID;   // And who performs delete???
}


void LeftNeighbourImpl::NeigbourDied(const nodeID& reportingNodeID) {
    cout << "Recieved message NeigbourDied from left neighbour" << endl;
}


void LeftNeighbourImpl::UpdateRightNode(const nodeID& newNodeID) {
    cout << "Recieved message UpdateRightNode from left neighbour" << endl;
}


void LeftNeighbourImpl::UpdateLeftNode(const nodeID& newNodeID) {
    cout << "Recieved message UpdateLeftNode from left neighbour" << endl;
}


void LeftNeighbourImpl::Boomerang(const blob& data) {
    cout << "Recieved message Boomerang from left neighbour" << endl;
}


/*****************************************************************/
//  RightNeighbour

nodeID* RightNeighbourImpl::ConnectAsLeftNode(const nodeID& newNodeID) {

    cout << "Recieved message ConnectAsLeftNode from right neighbour" << endl;

    // this cannot happen - don't know what to do
    throw "From right nieghbour came connecting request. This is not supported yet!";
}


void RightNeighbourImpl::NeigbourDied(const nodeID& reportingNodeID) {
    cout << "Recieved message NeigbourDied from right neighbour" << endl;
}


void RightNeighbourImpl::UpdateRightNode(const nodeID& newNodeID) {
    cout << "Recieved message UpdateRightNode from right neighbour" << endl;
    networkModule->changeRightNeighbour(newNodeID);
}


void RightNeighbourImpl::UpdateLeftNode(const nodeID& newNodeID) {
    cout << "Recieved message UpdateLeftNode from right neighbour" << endl;
}


void RightNeighbourImpl::Boomerang(const blob& data) {
    cout << "Recieved message Boomerang from right neighbour" << endl;
}
