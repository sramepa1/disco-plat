#include "Synchronization.h"

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
            cout << "I may be alone, but I have a new optimum: " << comp->getOptimum() << endl;
        }
        #endif
        return;
    }

    pthread_mutex_lock(&syncMutex);

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
    cout << "Accepting solution from circle with optimum " << circleSolutionOpt << endl;
#endif

        comp->setSolution(circleSolutionOpt, circleConfiguration, circleSolutionAbsolute);
        mySolutionOpt = circleSolutionOpt;

        haveMySolution = true;
    }

    if(send) {

#ifdef VERBOSE
        cout << "Broadcasting my solution with optimum " << mySolutionOpt << endl;
#endif

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
        if(splitSuccesful && comp->splitWork(unit)) {
            myColor = BLACK;

            blob message;
            message.sourceNode = networkModule->getMyID();
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

            rightNb->Boomerang(message);

#ifdef VERBOSE
            cout << "Assigning WORK to " << message.asignee.identifier << endl;
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
            cout << "Declining WORK REQUEST to " << message.sourceNode.identifier << endl;
#endif
        }

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
    cout << "I have nothing to do ... Requesting some work" << endl;
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
        cout << "Working thread awaken" << endl;
#endif
        return true;
    }


    // tokenized termination detection
    terminating = true;

    if(terminationToken) {
        sendTerminationToken();
    } else {
#ifdef VERBOSE
        cout << "I am termination leader - initiating termination protocol" << endl;
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
            cout << "Termination token send" << endl;
#endif

            pthread_cond_wait(&idleCondition, &syncMutex);

            if(commandTerminate) {
#ifdef VERBOSE
                cout << "Executing my termination" << endl;
#endif
                pthread_mutex_unlock(&syncMutex);
                return false;
            }

            if(recievedColor == WHITE) {
#ifdef VERBOSE
                cout << "Sending command to terminate the computation" << endl;
#endif

                blob message;
                message.sourceNode = networkModule->getMyID();
                message.computationID = computationID;
                message.messageType = TERMINATE;

                rightNb->Boomerang(message);

#ifdef VERBOSE
                cout << "Executing my termination" << endl;
#endif
                pthread_mutex_unlock(&syncMutex);
                return false;
            }
        }
    }

#ifdef VERBOSE
    cout << "Waiting for termnation command" << endl;
#endif

    while(!commandTerminate) {
        pthread_cond_wait(&idleCondition, &syncMutex);
    }

#ifdef VERBOSE
    cout << "Executing my termination" << endl;
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
    cout << "Accepting assigned WORK from " << data.sourceNode.identifier << endl;
#endif

    workReciewed = true;
    pthread_cond_signal(&idleCondition);

    pthread_mutex_unlock(&syncMutex);
}

// TODO: Lamport timestamps?
void Synchronization::updateWorkCache(string& identifier, WorkUnit& work, string& originalOwner) {
    if(originalOwner.empty()) {
        workAssignments.erase(originalOwner);  // De-zombify
    }
    workAssignments[identifier] = work;
}


void Synchronization::informNoAssignment() {
    pthread_mutex_lock(&syncMutex);

#ifdef VERBOSE
    cout << "WORK request declined" << endl;
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
    cout << "Token retuned with color " << (recievedColor == WHITE ? "WHITE" : "BLACK")  << endl;
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
    cout << "More leaders detected - loosing my leadership" << endl;
#endif
                terminationLeader = false;
                sendTerminationToken();
            } else {
#ifdef VERBOSE
    cout << "More leaders detected - keeping my leadership" << endl;
#endif
            }


        } else {
            sendTerminationToken();
        }
    } else {
#ifdef VERBOSE
    cout << "Saving token till my work is finished" << endl;
#endif
    }

    pthread_mutex_unlock(&syncMutex);
}


void Synchronization::informTerminate() {
    pthread_mutex_lock(&syncMutex);

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
    cout << "Sending token with color " << ( (color) message.slotA == WHITE ? "WHITE" : "BLACK")  << endl;
#endif

    rightNb->Boomerang(message);

    myColor = WHITE;
}

