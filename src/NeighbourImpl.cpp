#include "NeighbourImpl.h"
#include "NeighbourIface.h"

#include "Network.h"
#include "globals.h"

#include <iostream>

using namespace std;
using namespace disco_plat;

/*****************************************************************/
//  LeftNeighbour

void LeftNeighbourImpl::ConnectAsLeftNode(const nodeID& newNodeID, nodeID_out oldLeftNodeID) {
    cout << "Recieved message ConnectAsLeftNode from left neighbour" << endl;
    oldLeftNodeID = new nodeID(networkModule->getLeftID());
    networkModule->changeLeftNeighbour(newNodeID);
}


void LeftNeighbourImpl::NeigbourDied(const nodeID& reportingNodeID) {
    cout << "Recieved message NeigbourDied from left neighbour" << endl;
}


void LeftNeighbourImpl::UpdateRightNode(const nodeID& newNodeID) {
    cout << "Recieved message UpdateRightNode from left neighbour" << endl;
    networkModule->changeRightNeighbour(newNodeID);
}


void LeftNeighbourImpl::UpdateLeftNode(const nodeID& newNodeID) {
    cout << "Recieved message UpdateLeftNode from left neighbour" << endl;
    networkModule->changeLeftNeighbour(newNodeID);
}


void LeftNeighbourImpl::Boomerang(const blob& data) {
    cout << "Recieved message Boomerang from left neighbour" << endl;

    //TODO logiku pro zpracovani dat

    RightNeighbourIface& right = networkModule->getMyRightInterface();
    right.Boomerang(data);

    cout << "Sent message Boomerang to right neighbour" << endl;
}


/*****************************************************************/
//  RightNeighbour

void RightNeighbourImpl::ConnectAsLeftNode(const nodeID& newNodeID, nodeID_out oldLeftNodeID) {

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
    networkModule->changeLeftNeighbour(newNodeID);
}


void RightNeighbourImpl::Boomerang(const blob& data) {

    cout << "Recieved message Boomerang from right neighbour" << endl;

    // this cannot happen - don't know what to do
    throw "From right nieghbour came boomerang message. This is forbidden since boomerang goes in right-left direction only!";
}






