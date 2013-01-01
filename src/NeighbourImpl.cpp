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

#ifdef VERBOSE
                cout << "Message type is PING" << endl;
#endif

                break;

            case WORK_REQUEST :

#ifdef VERBOSE
                cout << "Message type is WORK_REQUEST" << endl;
#endif

                // TODO recovery and cache

                // request for my computatin ?
                if(currentSyncModule->getComputatuonID() == data.computationID)
                {
                    if(originMe) // my own rejected request
                    {
                        currentSyncModule->informNoAssignment();
                    }
                    else if(currentSyncModule->hasWorkToSplit()) // request for me
                    {
                        currentSyncModule->informRequest(data.sourceNode);
                        sendFurther = false;

#ifdef VERBOSE
                        cout << "Message WORK_REQUEST accepted for processing" << endl;
#endif
                    }
                }

                break;

            case WORK_ASSIGNMET :

#ifdef VERBOSE
                cout << "Message type is WORK_ASSIGNMET" << endl;
#endif

                // TODO recovery and cache

                if(data.asignee.identifier == networkModule->getMyID().identifier) {
                    currentSyncModule->informAssignment(data);
#ifdef VERBOSE
                    cout << "Message WORK_ASSIGNMET accepted for processing" << endl;
#endif
                }

                if(originRightNeighbour) {
                    sendFurther = false;
                }

                break;

            case RESULT : 

#ifdef VERBOSE
                cout << "Message type is RESULT" << endl;
#endif

                if(currentSyncModule->getComputatuonID() == data.computationID) {
                    currentSyncModule->informResult(data.slotA, data.charDataSequence);
#ifdef VERBOSE
                    cout << "Message RESULT accepted for processing" << endl;
#endif
                }

                if(originRightNeighbour) {
                    sendFurther = false;
                }

                break;

            case TERMINATION_TOKEN :

#ifdef VERBOSE
                cout << "Message type is TERMINATION_TOKEN" << endl;
#endif

                break;

            case TERMINATE :

#ifdef VERBOSE
                cout << "Message type is TERMINATE" << endl;
#endif

                currentSyncModule->informTerminate();
                break;

            default :
                break;
        }

    }

    ////////// Not yet fully started
    switch(data.messageType) {

        case TERMINATE :

#ifdef VERBOSE
            cout << "Message type is TERMINATE" << endl;
#endif

            if(currentSyncModule == NULL) {
                cout << "Recieved TERMINATE command when not yet fully initialized. Hard exit process." << endl;
                exit(ERR_COMMAND_TERMINATE);
            }

            break;

        case FREE_ID_SEARCH :

#ifdef VERBOSE
            cout << "Message type is FREE_ID_SEARCH" << endl;
#endif

            if(originMe) {
                //wake repository
                repo->awakeFreeID(data.slotA);

            } else {
                //change to my maxID if greater
                if(repo->getMaxID() > data.slotA) {
                    myData.slotA = repo->getMaxID();
                }
            }

            break;

        case INSTANCE_ANNOUNCEMENT :

#ifdef VERBOSE
            cout << "Message type is INSTANCE_ANNOUNCEMENT" << endl;
#endif

        /*    if(!originMe) { */
                repo->newData(data.computationID, string(data.dataStringA), string(data.dataStringB), false);

                if(data.slotA == BLOB_SA_IA_INIT_RESUME) {
                    repo->awakeInit();
                }

                if(originRightNeighbour) {
                    sendFurther = false;
                }
        //    }
#ifdef VERBOSE
       /*     else {
                cout << "Message was sent by myself. Ignoring." << endl;
            }*/
#endif

            break;

        default :
            break;
    }


    if(sendFurther) {
        RightNeighbourIface& right = networkModule->getMyRightInterface();
        right.Boomerang(myData);

#ifdef VERBOSE
        cout << "Boomerang was sent to right neighbour" << endl;
#endif

    }
#ifdef VERBOSE
    else {
        cout << "Boomerang ends here. No sending." << endl;
    }
#endif

}


void LeftNeighbourImpl::AbortingBoomerang() {
    cout << "Recieved message AbortingBoomerang from left neighbour" << endl;
    networkModule->getMyRightInterface().AbortingBoomerang();
}

/*****************************************************************/
//  RightNeighbour

void RightNeighbourImpl::BuildNetAndRequestData(const nodeID& newNeighbourID) {
    cout << "Recieved message BuildNetAndRequestData from right neighbour" << endl;
    networkModule->changeRightNeighbour(newNeighbourID);

#ifdef VERBOSE
        cout << "Sending all instance data to right neighbour" << endl;
#endif

    repo->sendAllData();

#ifdef VERBOSE
        cout << "Sending data was completed" << endl;
#endif
}


void RightNeighbourImpl::NodeDied(const nodeID& reportingNodeID, const nodeID& deadNodeID) {
    cout << "Recieved message NodeDied from right neighbour" << endl;
    networkModule->setDeadNodeID(reportingNodeID, deadNodeID);

    // TODO: check if I gave work to dead node

    networkModule->getMyLeftInterface().NeighbourDied(reportingNodeID, deadNodeID);
}


void RightNeighbourImpl::RebuildNetwork(const nodeID& newNeighbourID) {
    cout << "Recieved message RebuildNetwork from right neighbour" << endl;
    networkModule->cleanQueue();
    networkModule->changeRightNeighbour(newNeighbourID);
    networkModule->repairNetwork();
}
