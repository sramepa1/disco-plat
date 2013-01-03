#include "NeighbourImpl.h"
#include "NeighbourIface.h"

#include "globals.h"
#include "NeighbourIface.h"
#include "Network.h"
#include "Repository.h"
#include "Synchronization.h"

#include <iostream>

using namespace std;
using namespace disco_plat;
using namespace CORBA;


/*****************************************************************/
//  LeftNeighbour

void LeftNeighbourImpl::ConnectAsLeftNode(const nodeID& newNodeID, nodeID_out oldLeftNodeID) {
    repo->getOutput() << "Recieved message ConnectAsLeftNode from left neighbour" << endl;

    if(strlen(newNodeID.algorithm) !=0 && strcmp(newNodeID.algorithm, networkModule->getMyID().algorithm) != 0) {
        throw ConnectionError("Used algorithms does not match!");
    }

    oldLeftNodeID = new nodeID(networkModule->getLeftID());
    networkModule->changeLeftNeighbour(newNodeID);
}


void LeftNeighbourImpl::Boomerang(const blob& data) {

    // time update
    repo->timeColision(data.timestamp);

    // circle tracing
    blob myData(data);

    bool originMe = false;
    bool originRightNeighbour = false;
    bool sendFurther = true;

    if(strcmp(data.sourceNode.identifier, networkModule->getMyID().identifier) == 0) {
        sendFurther = false;
        originMe = true;
    }

    if(strcmp(data.sourceNode.identifier, networkModule->getRightID().identifier) == 0) {
        originRightNeighbour = true;
    }


    ////////// General boomerang types
    switch(data.messageType) {

        case PING :

#ifdef VERBOSE
            repo->getOutput() << "Recieved message Boomerang of type  PING" << endl;
#endif
            sendFurther = false;

            break;

        case FREE_ID_SEARCH :

#ifdef VERBOSE
            repo->getOutput() << "Recieved message Boomerang of type  FREE_ID_SEARCH" << endl;
#endif

            if(originMe) {
                //wake repository
                repo->awakeFreeID(data.slotA);

            } else {
                repo->informSearchID(data.slotA, data.sourceNode);
            }

            break;


        case NODE_ANNOUNCEMENT :

#ifdef VERBOSE
            repo->getOutput() << "Recieved message Boomerang of type  NODE_ANNOUNCEMENT" << endl;
#endif

            {
                string s1(data.dataStringA);
                repo->addLiveNode(s1);

                if(data.slotA == 2) {
                    string s2(data.dataStringB);
                    repo->addLiveNode(s2);
                }

                if(originMe && s1.compare(networkModule->getMyID().identifier) == 0) {
                    repo->setLiveNodeCacheConsistency(true);
                    repo->awakeInit();
                }

            }

            break;


        case INSTANCE_ANNOUNCEMENT :

#ifdef VERBOSE
            repo->getOutput() << "Recieved message Boomerang of type  INSTANCE_ANNOUNCEMENT" << endl;
#endif

            repo->newData(data.computationID, string(data.dataStringA), string(data.dataStringB), false);

            if(data.slotA == BLOB_SA_IA_INIT_RESUME) {
                repo->broadcastMyID();
            }

            if(originRightNeighbour) {
                sendFurther = false;
            }

            break;

        case NETWORK_REBUILT: {
#ifdef VERBOSE
            repo->getOutput() << "Recieved message Boomerang of type  NETWORK_REBUILT" << endl;
#endif
            if(sendFurther) {
                networkModule->getMyRightInterface().Boomerang(myData);
            }
            sendFurther = false;

            set<string> liveNodesSet;
            for(unsigned i = 0; i < data.nodeIDSequence.length(); ++i) {
                liveNodesSet.insert((const char*)data.nodeIDSequence[i].identifier);
            }
            repo->setLiveNodes(liveNodesSet);

            set<unsigned> compIDSet;
            for(unsigned i = 0; i < data.longDataSequence.length(); ++i) {
                compIDSet.insert(data.longDataSequence[i]);
            }
            repo->setSurvivingComputations(compIDSet);

            // zombify my request if any
            repo->lockCurrentSyncModule();

            Synchronization* currentSyncModule = repo->getCurrentSyncModule();
            if(currentSyncModule != NULL) {
                currentSyncModule->informNetworkRebuild();
            }

            repo->unlockCurrentSyncModule();

            break;
        }

        default :
            break;
    }


    ////////// killing zombie boomerang
    {
        string s(data.sourceNode.identifier);
        if(!repo->isAlive(s)) {
#ifdef VERBOSE
            repo->getOutput() << "Dropping dead boomerang with origin " << data.sourceNode.identifier << endl;
#endif
            return;
        }
    }

    ////////// Synchronizing boomerang types
    repo->lockCurrentSyncModule();
    Synchronization* currentSyncModule = repo->getCurrentSyncModule();

    if(currentSyncModule != NULL) {

        switch(data.messageType) {

            case WORK_REQUEST :

#ifdef VERBOSE
                repo->getOutput() << "Recieved message Boomerang of type  WORK_REQUEST" << endl;
#endif

                // request for my computatin ?
                if(currentSyncModule->getComputationID() == data.computationID)
                {
                    if(originMe) // my own rejected request
                    {
                        if(!currentSyncModule->isZombifying()) {
                            currentSyncModule->informNoAssignment();
                        } else {
#ifdef VERBOSE
                            repo->getOutput() << "Zombifying under way - WORK DENIAL dropped" << endl;
#endif
                        }
                    }
                    else if(currentSyncModule->hasWorkToSplit()) // request for me
                    {
                        currentSyncModule->informRequest(data.sourceNode);
                        sendFurther = false;
                    }
                }

                break;

            case WORK_ASSIGNMET :
#ifdef VERBOSE
                repo->getOutput() << "Recieved message Boomerang of type  WORK_ASSIGNMET" << endl;
#endif


                if(currentSyncModule->getComputationID() == data.computationID) {
                    // is that work for me
                    if(strcmp(data.asignee.identifier, networkModule->getMyID().identifier) == 0) {

                        if(!currentSyncModule->isZombifying()) {
                            // check original owner
                            string zombieIdentifer(data.dataStringA);
                            if(!zombieIdentifer.empty()) {
                                currentSyncModule->killZombie(zombieIdentifer);
                                #ifdef VERBOSE
                                repo->getOutput() << "Received self-assigned zombie work, originally owned by "
                                                  << zombieIdentifer << endl;
                                #endif
                            }

                            // take it
                            currentSyncModule->informAssignment(data);

                        } else {
#ifdef VERBOSE
                            repo->getOutput() << "Zombifying under way - WORK ASSIGNMET dropped" << endl;
#endif
                        }

                    } else {

                        // update cache and/or this message
                        currentSyncModule->updateWorkCache(myData);
                    }
                }

                break;

            case RESULT :

#ifdef VERBOSE
                repo->getOutput() << "Recieved message Boomerang of type  RESULT" << endl;
#endif

                if(currentSyncModule->getComputationID() == data.computationID) {
                    currentSyncModule->informResult(myData);
                }

                break;

            case TERMINATION_TOKEN :

#ifdef VERBOSE
                repo->getOutput() << "Recieved message Boomerang of type  TERMINATION_TOKEN" << endl;
#endif

                if(currentSyncModule->getComputationID() == data.computationID) {
                    sendFurther = false;

                    if(originMe) {
                        currentSyncModule->informMyToken(data);
                    } else {
                        currentSyncModule->informForeignToken(data);
                    }
                }

                break;

            case TERMINATE :

#ifdef VERBOSE
                repo->getOutput() << "Recieved message Boomerang of type  TERMINATE" << endl;
#endif

                if(currentSyncModule->getComputationID() == data.computationID) {
                    currentSyncModule->informTerminate(data);
                } else {
#ifdef VERBOSE
                repo->getOutput() << "Deleting terminated computation from cache" << endl;
#endif
                    repo->destroyComputation(data.computationID);
                }

                break;

            default :
                break;

            case ZOMBIFY :

#ifdef VERBOSE
                repo->getOutput() << "Recieved message Boomerang of type  ZOMBIFY" << endl;
#endif

                if(currentSyncModule->getComputationID() == data.computationID) {

                    if(originMe) {
                        currentSyncModule->informZombifyFinish();
                    } else {
                        set<string> deadIdentifiers;
                        string s(data.asignee.identifier);
                        deadIdentifiers.insert(s);
                        currentSyncModule->zombify(deadIdentifiers);
                    }
                }


                break;

        }

        if(sendFurther) {
            currentSyncModule->pingReset();
        }
    }

    repo->unlockCurrentSyncModule();

    if(sendFurther) {
        RightNeighbourIface& right = networkModule->getMyRightInterface();
        right.Boomerang(myData);
    }

}


void LeftNeighbourImpl::AbortingBoomerang() {
    repo->getOutput() << "Recieved message AbortingBoomerang from left neighbour" << endl;
    networkModule->getMyRightInterface().AbortingBoomerang();
}

/*****************************************************************/
//  RightNeighbour

void RightNeighbourImpl::BuildNetAndRequestData(const nodeID& newNeighbourID) {
    repo->getOutput() << "Recieved message BuildNetAndRequestData from right neighbour" << endl;
    networkModule->changeRightNeighbour(newNeighbourID);

#ifdef VERBOSE
        repo->getOutput() << "Sending all instance data to right neighbour" << endl;
#endif

    repo->sendAllData();

#ifdef VERBOSE
        repo->getOutput() << "Sending data was completed" << endl;
#endif
}


void RightNeighbourImpl::NodeDied(const nodeID& reportingNodeID, SequenceTmpl<nodeID, MICO_TID_DEF> liveNodes,
                                  SequenceTmpl<Long, MICO_TID_DEF> compIDs) {

    repo->getOutput() << "Recieved message NodeDied from right neighbour" << endl;
    cout << "report ID: " << reportingNodeID.identifier << endl;

    SequenceTmpl<nodeID, MICO_TID_DEF> newNodeSequence;
    SequenceTmpl<Long, MICO_TID_DEF> newCompSequence;
    newNodeSequence.length(liveNodes.length() + 1);
    newCompSequence.length(liveNodes.length() + 1);

    for(unsigned i = 0; i < liveNodes.length(); ++i) {
        newNodeSequence[i] = liveNodes[i];
        newCompSequence[i] = compIDs[i];
    }
    newNodeSequence[liveNodes.length()] = networkModule->getMyID();
    newCompSequence[liveNodes.length()] = repo->getCurrentComputationID();

    networkModule->setDataForRebuilding(reportingNodeID, newNodeSequence, newCompSequence);
    networkModule->getMyLeftInterface().NeighbourDied(reportingNodeID, newNodeSequence, newCompSequence);
}


void RightNeighbourImpl::RebuildNetwork(const nodeID& newNeighbourID) {
    repo->getOutput() << "Recieved message RebuildNetwork from right neighbour" << endl;
    networkModule->changeRightNeighbour(newNeighbourID);
    networkModule->repairNetwork();
}
