#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include "globals.h"
#include "NeighbourIface.h"
#include "Computation.h"

#include <pthread.h>


enum SyncState { WORKING, SYNCHRONIZING, IDLING, TERMINATING };


class Synchronization
{

    LeftNeighbourIface* leftNb;
    RightNeighbourIface* rightNb;

    unsigned int computationID;

    Computation* comp;

    SyncState state;
    pthread_mutex_t stateMutex;


    // DISABLED
    Synchronization(const Synchronization&) {}

public:
    Synchronization(Computation* comp);
    ~Synchronization();

    ///////////// algorithm interface

    /**
     * Checks if threre are any incoming messages and then calls their handlers.
     */
    void synchronize();

    /**
     * Blocks until new work arrives or computation terminates.
     */
    bool isWorkAvailable();


    ///////////// network interface

    void informAssignment(const char* data, int dataLenght, unsigned int computationID);
    void informRequest(disco_plat::nodeID requesteeID);
    void informResult(const char* data, int dataLenght);
    void informTerminate();

};

#endif // SYNCHRONIZATION_H
