#include "Synchronization.h"

#include "../build/Interface.h"

using namespace std;
using namespace disco_plat;

Synchronization::Synchronization(Computation* comp, unsigned int id) : comp(comp), computationID(id)
{
    leftNb = &networkModule->getMyLeftInterface();
    rightNb = &networkModule->getMyRightInterface();

    pthread_mutex_init(&stateMutex, NULL);

    haveNewSolutionFromCircle = false;
    haveMySolution = false;
}

Synchronization::~Synchronization()
{
    pthread_mutex_destroy(&stateMutex);
}


void Synchronization::synchronize() {

    pthread_mutex_lock(&stateMutex);
    state = SYNCHRONIZING;
    pthread_mutex_unlock(&stateMutex);

    // solution
    bool send;
    bool accept;

    if(haveNewSolutionFromCircle) {
        accept = true;
    } else {
        accept = false;
    }

    if(comp->hasNewSolution()) {
        pair<opt_t, vector<char> > tmp = comp->getSolution();
        mySolution = tmp.first;
        myConfiguration = tmp.second;

        haveMySolution = true;
        send = true;
    } else {
        send = false;
    }

    if(accept && haveMySolution) {
        if(comp->isBetter(mySolution, newSolutionFromCircle)) {
            accept = false;
        } else {
            send = false;
        }
    }

    if(accept) {

#ifdef VERBOSE
    cout << "Accepting solution from circle" << endl;
#endif

        comp->setSolution(newSolutionFromCircle, circleConfiguration, false); // TODO how to get is trivial ?
    }

    if(send) {

#ifdef VERBOSE
    cout << "Bradcasting my solution" << endl;
#endif

        // let the ring know
        blob message;
        message.sourceNode = networkModule->getMyID();
        message.computationID = computationID;
        message.messageType = RESULT;

        message.slotA = (unsigned int) mySolution;

        blob::_charDataSequence_seq data(myConfiguration.size());
        for(unsigned int i = 0; i < myConfiguration.size(); ++i) {
            data[i] = myConfiguration[i];
        }

        message.charDataSequence = data;

        rightNb->Boomerang(message);

    }

    // work requests



    // flags reset
    haveNewSolutionFromCircle = false;

    pthread_mutex_lock(&stateMutex);
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

void Synchronization::informResult(const char* data, int dataLenght) {

}


void Synchronization::informTerminate() {
    pthread_mutex_lock(&stateMutex);
    state = TERMINATING;
    pthread_mutex_unlock(&stateMutex);
}


