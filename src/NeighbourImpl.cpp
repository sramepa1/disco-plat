#include "NeighbourImpl.h"
#include "NeighbourIface.h"

#include "globals.h"
#include "Network.h"
#include "Repository.h"
#include "Synchronization.h"

#include <iostream>

using namespace std;
using namespace disco_plat;


/*****************************************************************/
//  LeftNeighbour

void LeftNeighbourImpl::ConnectAsLeftNode(const nodeID& newNodeID, nodeID_out oldLeftNodeID) {
    cout << "Recieved message ConnectAsLeftNode from left neighbour" << endl;

    if(strlen(newNodeID.algorithm) !=0 && strcmp(newNodeID.algorithm, networkModule->getMyID().algorithm) != 0) {
        throw ConnectionError("Used algorithms does not match!");
    }

    oldLeftNodeID = new nodeID(networkModule->getLeftID());
    networkModule->changeLeftNeighbour(newNodeID);
}


void LeftNeighbourImpl::NeigbourDied(const nodeID& reportingNodeID, const nodeID& deadNodeID) {
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

    bool originMe = false;
    bool sendFurther = true;

    if(data.sourceNode.identifier == networkModule->getMyID().identifier) {
        sendFurther = false;
        originMe = true;
    }

    ////////// Fully started
    if(currentSyncModule != NULL) {

        switch(data.messageType) {
            case PING :
                break;

            case WORK_REQUEST :

                break;

            case WORK_ASSIGNMET :

                break;

            case RESULT :
                //currentSyncModule->informResult(data.charDataSequence .get_buffer(), data.data.length());
                break;

            case TERMINATION_TOKEN :

                break;

            case TERMINATE :
                currentSyncModule->informTerminate();
                break;

            default :
                break;
        }

    }

    ////////// Not yet fully started
    switch(data.messageType) {

        case TERMINATE :
            if(currentSyncModule == NULL) exit(ERR_COMMAND_TERMINATE);
            break;

        case ID_SEARCH :

            if(originMe) {
                //TODO set info to storage
            } else {
                //TODO change to my ID if greater
            }

            break;

        case INSTANCE_ANNOUNCEMENT :

            repo->newData(data.computationID, string(data.dataStringA), string(data.dataStringB), false);

            if(data.slotA == BLOB_SA_IA_INIT_RESUME) {
                repo->awakeInit();
            }

            break;

        default :
            break;
    }


    if(sendFurther) {
        RightNeighbourIface& right = networkModule->getMyRightInterface();
        right.Boomerang(data);
    }

    cout << "Sent message Boomerang to right neighbour" << endl;
}


/*****************************************************************/
//  RightNeighbour

void RightNeighbourImpl::RequestComputationalData(const nodeID& destinationID) {
    cout << "Recieved message RequestComputationalData from right neighbour" << endl;
}


void RightNeighbourImpl::NeigbourDied(const nodeID& reportingNodeID, const nodeID& deadNodeID) {
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
