#include "Synchronization.h"
#include "Repository.h"

#include "../build/Interface.h"

using namespace std;
using namespace disco_plat;

Synchronization::Synchronization(Computation* comp, unsigned int id) : comp(comp), computationID(id)
{
    leftNb = &networkModule->getMyLeftInterface();
    rightNb = &networkModule->getMyRightInterface();

    pthread_mutex_init(&stateMutex, NULL);
    pthread_mutex_init(&syncMutex, NULL);
    pthread_cond_init(&idleCondition, NULL);

    haveNewCirlceSolution = false;
    haveMySolution = false;
    isWorkingState = true;
    splitSuccesful = true;

    terminating = false;
    terminationLeader = false;
    commandTerminate = false;
    terminationToken = false;

    pingCounter = 0;

    myColor = WHITE;
}

Synchronization::~Synchronization()
{
    pthread_mutex_destroy(&stateMutex);
    pthread_mutex_destroy(&syncMutex);
    pthread_cond_destroy(&idleCondition);
}


void Synchronization::synchronize() {

    if(networkModule->isSingle()) {
        #ifdef VERBOSE
        if(comp->hasNewSolution()) {
            comp->getSolution();
            repo->getOutput() << "I may be alone, but I have a new optimum: " << comp->getOptimum() << endl;
        }
        #endif
        return;
    }

    pthread_mutex_lock(&syncMutex);

    bool ping = true;

    // solution
    bool send;
    bool accept;

    if(haveNewCirlceSolution) {
        accept = true;
    } else {
        accept = false;
    }

    if(comp->hasNewSolution()) {
        pair<opt_t, vector<char> > tmp = comp->getSolution();
        mySolutionOpt = tmp.first;
        myConfiguration = tmp.second;
        mySolutionAbsolute = comp->isSolutionAbsolute();

        haveMySolution = true;
        send = true;
    } else {
        send = false;
    }

    if(accept && haveMySolution) {
        if(comp->isBetter(mySolutionOpt, circleSolutionOpt)) {
            accept = false;
        } else {
            send = false;
        }
    }

    if(accept) {

#ifdef VERBOSE
    repo->getOutput() << "Accepting solution from circle with optimum " << circleSolutionOpt << endl;
#endif

        comp->setSolution(circleSolutionOpt, circleConfiguration, circleSolutionAbsolute);
        mySolutionOpt = circleSolutionOpt;

        haveMySolution = true;
    }

    if(send) {

#ifdef VERBOSE
        repo->getOutput() << "Broadcasting my solution with optimum " << mySolutionOpt << endl;
#endif
        ping = false;

        // let the ring know
        blob message;
        message.sourceNode = networkModule->getMyID();
        message.computationID = computationID;
        message.messageType = RESULT;

        message.slotA = (unsigned int) mySolutionOpt;
        message.slotB = (unsigned int) mySolutionAbsolute;

        message.charDataSequence.length(myConfiguration.size());
        for(unsigned int i = 0; i < myConfiguration.size(); ++i) {
            message.charDataSequence[i] = myConfiguration[i];
        }

        rightNb->Boomerang(message);

        circleSolutionOpt = mySolutionOpt;
    }

    // work requests
    while(!workRequests.empty()) {
        ping = false;

        if(splitSuccesful && comp->splitWork(unit)) {
            myColor = BLACK;

            blob message;
            message.sourceNode = workRequests.front();
            message.computationID = computationID;
            message.messageType = WORK_ASSIGNMET;

            message.asignee = workRequests.front();
            workRequests.pop_front();

            message.slotA = unit.depth;
            message.slotB = unit.instanceSize;

            message.charDataSequence.length(unit.configStackVector.size());
            for(unsigned int i = 0; i < unit.configStackVector.size(); ++i) {
                message.charDataSequence[i] = unit.configStackVector[i];
            }

            message.longDataSequence.length(unit.intervalStackVector.size());
            for(unsigned int i = 0; i < unit.intervalStackVector.size(); ++i) {
                message.longDataSequence[i] = unit.intervalStackVector[i];
            }

            message.dataStringA = "";

            rightNb->Boomerang(message);

#ifdef VERBOSE
            repo->getOutput() << "Assigning WORK to " << message.asignee.identifier << endl;
#endif

        } else {
            splitSuccesful = false;

            blob message;
            message.sourceNode = workRequests.front();
            workRequests.pop_front();
            message.computationID = computationID;
            message.messageType = WORK_REQUEST;

            rightNb->Boomerang(message);

#ifdef VERBOSE
            repo->getOutput() << "Declining WORK REQUEST to " << message.sourceNode.identifier << endl;
#endif
        }

    }

    // pinging
    if(ping) {
        if(pingCounter > PING_COUNTER_MAX) {

#ifdef VERBOSE
            repo->getOutput() << "PINGING cause no message send for a long time" << endl;
#endif

            pingCounter = 0;

            blob message;
            message.sourceNode = networkModule->getMyID();
            message.computationID = computationID;
            message.messageType = PING;

            rightNb->Boomerang(message);

        } else {
            pingCounter++;
        }
    } else {
        pingCounter = 0;
    }

    // flags reset
    haveNewCirlceSolution = false;
    pthread_mutex_unlock(&syncMutex);

}


bool Synchronization::isWorkAvailable() {

    if(networkModule->isSingle()) {
        return false;
    }

#ifdef VERBOSE
    repo->getOutput() << "I have nothing to do ... Requesting some work" << endl;
#endif

    pthread_mutex_lock(&stateMutex);
    isWorkingState = false;
    pthread_mutex_unlock(&stateMutex);

    // ask for more work

    pthread_mutex_lock(&syncMutex);

    blob message;
    message.sourceNode = networkModule->getMyID();
    message.computationID = computationID;
    message.messageType = WORK_REQUEST;

    rightNb->Boomerang(message);

    workReciewed = false;
    pthread_cond_wait(&idleCondition, &syncMutex);

    if(workReciewed) {
        pthread_mutex_unlock(&syncMutex);

        pthread_mutex_lock(&stateMutex);
        isWorkingState = true;
        pthread_mutex_unlock(&stateMutex);

#ifdef VERBOSE
        repo->getOutput() << "Working thread awaken" << endl;
#endif
        return true;
    }


    // tokenized termination detection
    terminating = true;

    if(terminationToken) {
        sendTerminationToken();
    } else {
#ifdef VERBOSE
        repo->getOutput() << "I am termination leader - initiating termination protocol" << endl;
#endif

        terminationLeader = true;

        while(terminationLeader) {
            blob message;
            message.sourceNode = networkModule->getMyID();
            message.computationID = computationID;
            message.messageType = TERMINATION_TOKEN;
            message.slotA = (unsigned int) WHITE;

            rightNb->Boomerang(message);

#ifdef VERBOSE
            repo->getOutput() << "Termination token send" << endl;
#endif

            pthread_cond_wait(&idleCondition, &syncMutex);

            if(commandTerminate) {
#ifdef VERBOSE
                repo->getOutput() << "Executing my termination" << endl;
#endif
                pthread_mutex_unlock(&syncMutex);
                return false;
            }

            if(recievedColor == WHITE) {
#ifdef VERBOSE
                repo->getOutput() << "Sending command to terminate the computation" << endl;
#endif

                blob message;
                message.sourceNode = networkModule->getMyID();
                message.computationID = computationID;
                message.messageType = TERMINATE;

                message.slotA = (unsigned int) mySolutionOpt;

                message.charDataSequence.length(myConfiguration.size());
                for(unsigned int i = 0; i < myConfiguration.size(); ++i) {
                    message.charDataSequence[i] = myConfiguration[i];
                }

#ifdef VERBOSE
                repo->getOutput() << "Waiting for all nodes to terminate" << endl;
#endif

                rightNb->Boomerang(message);

                while(!commandTerminate) {
                    pthread_cond_wait(&idleCondition, &syncMutex);
                }

#ifdef VERBOSE
                repo->getOutput() << "Executing my termination" << endl;
#endif
                pthread_mutex_unlock(&syncMutex);
                return false;
            }
        }
    }

#ifdef VERBOSE
    repo->getOutput() << "Waiting for termnation command" << endl;
#endif

    while(!commandTerminate) {
        pthread_cond_wait(&idleCondition, &syncMutex);
    }

#ifdef VERBOSE
    repo->getOutput() << "Executing my termination" << endl;
#endif
    pthread_mutex_unlock(&syncMutex);
    return false;
}

void Synchronization::informAssignment(blob data) {
    pthread_mutex_lock(&syncMutex);

    unit.depth = data.slotA;
    unit.instanceSize = data.slotB;
    unit.configStackVector =  vector<char>(data.charDataSequence.get_buffer(), data.charDataSequence.get_buffer() + data.charDataSequence.length());
    unit.intervalStackVector = vector<int>(data.longDataSequence.get_buffer(), data.longDataSequence.get_buffer() + data.longDataSequence.length());

    comp->setWork(unit);

#ifdef VERBOSE
    repo->getOutput() << "Accepting assigned WORK from " << data.sourceNode.identifier << endl;
#endif

    workReciewed = true;
    pthread_cond_signal(&idleCondition);

    pthread_mutex_unlock(&syncMutex);
}

void Synchronization::updateWorkCache(string& identifier, uint64_t time, WorkUnit& work, string& originalOwner) {

#ifdef VERBOSE
    repo->getOutput() << "Updating work cache for " << identifier << endl;
#endif

    if(!originalOwner.empty()) {
#ifdef VERBOSE
        repo->getOutput() << "Dezombifying work cache from " << originalOwner << endl;
#endif
        workAssignments.erase(originalOwner);  // De-zombify
    }
    workAssignments[identifier] = make_pair(time, work);
}


void Synchronization::informNoAssignment() {
    pthread_mutex_lock(&syncMutex);

#ifdef VERBOSE
    repo->getOutput() << "WORK request declined" << endl;
#endif

    workReciewed = false;
    pthread_cond_signal(&idleCondition);

    pthread_mutex_unlock(&syncMutex);
}

void Synchronization::informRequest(disco_plat::nodeID requesteeID) {
    pthread_mutex_lock(&syncMutex);
    workRequests.push_back(requesteeID);
    pthread_mutex_unlock(&syncMutex);
}

void Synchronization::informResult(blob data) {
    pthread_mutex_lock(&syncMutex);

    if(comp->isBetter(data.slotA, circleSolutionOpt)) {
        vector<char> tmp(data.charDataSequence.length());
        for(unsigned int i = 0; i < data.charDataSequence.length(); ++i) {
            tmp[i] = data.charDataSequence[i];
        }

        circleSolutionOpt = data.slotA;
        circleConfiguration = tmp;
        circleSolutionAbsolute = (bool) data.slotB;

        haveNewCirlceSolution = true;
    }

    pthread_mutex_unlock(&syncMutex);
}


void Synchronization::informMyToken(blob data) {
    pthread_mutex_lock(&syncMutex);

    terminationToken = true;
    recievedColor = (color) data.slotA;

#ifdef VERBOSE
    repo->getOutput() << "Token retuned with color " << (recievedColor == WHITE ? "WHITE" : "BLACK")  << endl;
#endif

    pthread_cond_signal(&idleCondition);

    pthread_mutex_unlock(&syncMutex);
}

void Synchronization::informForeignToken(blob data) {
    pthread_mutex_lock(&syncMutex);

    terminationToken = true;
    recievedColor = (color) data.slotA;
    tokenOrigin = data.sourceNode;

    pthread_mutex_lock(&stateMutex);
    bool tmp = isWorkingState;
    pthread_mutex_unlock(&stateMutex);

    if(!tmp) {
        if(terminationLeader) {
            string me(networkModule->getMyID().identifier);
            string competitor(data.sourceNode.identifier);

            if(me.compare(competitor) > 0) {
#ifdef VERBOSE
    repo->getOutput() << "More leaders detected - loosing my leadership" << endl;
#endif
                terminationLeader = false;
                sendTerminationToken();
            } else {
#ifdef VERBOSE
    repo->getOutput() << "More leaders detected - keeping my leadership" << endl;
#endif
            }


        } else {
            sendTerminationToken();
        }
    } else {
#ifdef VERBOSE
    repo->getOutput() << "Saving token till my work is finished" << endl;
#endif
    }

    pthread_mutex_unlock(&syncMutex);
}


void Synchronization::informTerminate(blob data) {
    pthread_mutex_lock(&syncMutex);

    vector<char> tmp(data.charDataSequence.length());
    for(unsigned int i = 0; i < data.charDataSequence.length(); ++i) {
        tmp[i] = data.charDataSequence[i];
    }

    circleSolutionOpt = data.slotA;
    circleConfiguration = tmp;

    comp->setSolution(circleSolutionOpt, circleConfiguration, false);

    commandTerminate = true;
    pthread_cond_signal(&idleCondition);

    pthread_mutex_unlock(&syncMutex);
}

bool Synchronization::isWorking() {
    pthread_mutex_lock(&stateMutex);
    bool tmp = isWorkingState;
    pthread_mutex_unlock(&stateMutex);

    return tmp;
}

bool Synchronization::hasWorkToSplit() {
    pthread_mutex_lock(&stateMutex);
    bool tmp = isWorkingState && splitSuccesful;
    pthread_mutex_unlock(&stateMutex);

    return tmp;
}


void Synchronization::sendTerminationToken() {
    blob message;
    message.sourceNode = tokenOrigin;
    message.computationID = computationID;
    message.messageType = TERMINATION_TOKEN;

    if(recievedColor == BLACK) {
        message.slotA = BLACK;
    } else {
        message.slotA = (unsigned int) myColor;
    }

#ifdef VERBOSE
    repo->getOutput() << "Sending token with color " << ( (color) message.slotA == WHITE ? "WHITE" : "BLACK")  << endl;
#endif

    rightNb->Boomerang(message);

    myColor = WHITE;
}

