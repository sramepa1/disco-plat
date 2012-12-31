#include "Synchronization.h"

#include "../build/Interface.h"

using namespace std;
using namespace disco_plat;

Synchronization::Synchronization(Computation* comp, unsigned int id) : comp(comp), computationID(id)
{
    leftNb = &networkModule->getMyLeftInterface();
    rightNb = &networkModule->getMyRightInterface();

    pthread_mutex_init(&stateMutex, NULL);

    haveNewCirlceSolution = false;
    haveMySolution = false;
}

Synchronization::~Synchronization()
{
    pthread_mutex_destroy(&stateMutex);
}


void Synchronization::synchronize() {

    pthread_mutex_lock(&stateMutex);
    state = SYNCHRONIZING;

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

        comp->setSolution(circleSolutionOpt, circleConfiguration, false); // TODO how to get is trivial ?
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

        blob::_charDataSequence_seq data(myConfiguration.size());
        for(unsigned int i = 0; i < myConfiguration.size(); ++i) {
            data[i] = myConfiguration[i];
        }

        message.charDataSequence = data;

        rightNb->Boomerang(message);

        circleSolutionOpt = mySolutionOpt;
    }

    // work requests

    //TODO reply to work requests


    // flags reset
    haveNewCirlceSolution = false;

    state = WORKING;
    pthread_mutex_unlock(&stateMutex);

}


bool Synchronization::isWorkAvailable() {

    pthread_mutex_lock(&stateMutex);
    state = IDLING;
    pthread_mutex_unlock(&stateMutex);





    return false;

}

void Synchronization::informAssignment(const char* data, int dataLenght, unsigned int computationID) {
    pthread_mutex_lock(&stateMutex);
    state = WORKING;
    pthread_mutex_unlock(&stateMutex);



}

void Synchronization::informRequest(disco_plat::nodeID requesteeID) {

}

void Synchronization::informResult(unsigned int id, unsigned int optimum, blob::_charDataSequence_seq data) {
    pthread_mutex_lock(&stateMutex);

    if(id == computationID && comp->isBetter(optimum, circleSolutionOpt)) {
        vector<char> tmp(data.length());
        for(unsigned int i = 0; i < data.length(); ++i) {
            tmp[i] = data[i];
        }

        circleSolutionOpt = optimum;
        circleConfiguration = tmp;

        haveNewCirlceSolution = true;

#ifdef VERBOSE
        cout << "Received accepted for processing" << endl;
#endif
    }
#ifdef VERBOSE
    else {
        cout << "Received solution rejected; id is " << id << ", optimum is " << optimum << endl;
    }
#endif

    pthread_mutex_unlock(&stateMutex);
}


void Synchronization::informTerminate() {
    pthread_mutex_lock(&stateMutex);
    state = TERMINATING;
    pthread_mutex_unlock(&stateMutex);
}


