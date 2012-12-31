#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include "globals.h"
#include "NeighbourIface.h"
#include "Computation.h"

#include <pthread.h>
#include <vector>


enum SyncState { WORKING, SYNCHRONIZING, IDLING, TERMINATING };


class Synchronization
{
    Computation* comp;

    LeftNeighbourIface* leftNb;
    RightNeighbourIface* rightNb;

    unsigned int computationID;

    std::vector<char> circleConfiguration;
    opt_t circleSolutionOpt;
    bool haveNewCirlceSolution;

    std::vector<char> myConfiguration;
    opt_t mySolutionOpt;
    bool haveMySolution;

    SyncState state;
    pthread_mutex_t stateMutex;

    // DISABLED
    Synchronization(const Synchronization&) {}

public:
    Synchronization(Computation* comp, unsigned int id);
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
    void informResult(unsigned int id, unsigned int optimum, disco_plat::blob::_charDataSequence_seq data);
    void informTerminate();

};

#endif // SYNCHRONIZATION_H
