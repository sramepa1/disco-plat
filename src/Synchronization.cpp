#include "Synchronization.h"

#include "../build/Interface.h"

using namespace std;
using namespace disco_plat;

Synchronization::Synchronization(Computation* comp) : comp(comp)
{
    leftNb = &networkModule->getMyLeftInterface();
    rightNb = &networkModule->getMyRightInterface();

    pthread_mutex_init(&stateMutex, NULL);
}

Synchronization::~Synchronization()
{
    pthread_mutex_destroy(&stateMutex);
}


void Synchronization::synchronize() {

    pthread_mutex_lock(&stateMutex);
    if(TERMINATING) {
        // TODO manualy terminated
    }

    state = SYNCHRONIZING;
    pthread_mutex_unlock(&stateMutex);




    pthread_mutex_lock(&stateMutex);
    if(TERMINATING) {
        // TODO manualy terminated
    }

    state = WORKING;
    pthread_mutex_unlock(&stateMutex);

}


bool Synchronization::isWorkAvailable() {

    pthread_mutex_lock(&stateMutex);
    if(TERMINATING) {
        // TODO manualy terminated
    }

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


