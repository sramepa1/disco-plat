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


void LeftNeighbourImpl::Boomerang(const blob& data) {
    cout << "Recieved message Boomerang from left neighbour" << endl;

    blob myData(data);

    bool originMe = false;
    bool originRightNeighbour = false;
    bool sendFurther = true;

    if(data.sourceNode.identifier == networkModule->getMyID().identifier) {
        sendFurther = false;
        originMe = true;
    }

    if(data.sourceNode.identifier == networkModule->getRightID().identifier) {
        originRightNeighbour = true;
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

        case FREE_ID_SEARCH :

            if(originMe) {
                //wake repository
                repo->setMaxID(data.slotA);
                repo->awakeFreeID();

            } else {
                //change to my maxID if greater
                if(repo->getMaxID() > data.slotA) {
                    myData.slotA = repo->getMaxID();
                }
            }

            break;

        case INSTANCE_ANNOUNCEMENT :

            if(!originMe) {
                repo->newData(data.computationID, string(data.dataStringA), string(data.dataStringB), false);

                if(data.slotA == BLOB_SA_IA_INIT_RESUME) {
                    repo->awakeInit();
                }

                if(originRightNeighbour) {
                    sendFurther = false;
                }
            }

            break;

        default :
            break;
    }


    if(sendFurther) {
        RightNeighbourIface& right = networkModule->getMyRightInterface();
        right.Boomerang(myData);
        cout << "Sent message Boomerang to right neighbour" << endl;
    }
}


/*****************************************************************/
//  RightNeighbour

void RightNeighbourImpl::BuildNetAndRequestData(const nodeID& newNeighbourID) {
    cout << "Recieved message BuildNetAndRequestData from right neighbour" << endl;
    networkModule->changeRightNeighbour(newNeighbourID);

    // TODO: complete it
}


void RightNeighbourImpl::NodeDied(const nodeID& reportingNodeID, const nodeID& deadNodeID) {
    cout << "Recieved message NodeDied from right neighbour" << endl;
    networkModule->setReportNodeID(reportingNodeID);

    // TODO: check if I gave work to dead node

    networkModule->getMyLeftInterface().NeighbourDied(reportingNodeID, deadNodeID);
}


void RightNeighbourImpl::RebuildNetwork(const nodeID& newNeighbourID) {
    cout << "Recieved message RebuildNetwork from right neighbour" << endl;
    networkModule->changeRightNeighbour(newNeighbourID);
    networkModule->repairNetwork();
}
