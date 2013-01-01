#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include "globals.h"
#include "NeighbourIface.h"
#include "Computation.h"

#include <list>
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

    struct WorkUnit unit;

    bool isWorkingState;
    bool workReciewed;
    bool splitSuccesful;

    std::list<disco_plat::nodeID> workRequests;

    pthread_mutex_t stateMutex;
    pthread_mutex_t syncMutex;
    pthread_cond_t idleCondition;


    // DISABLED
    Synchronization(const Synchronization&) {}

public:
    Synchronization(Computation* comp, unsigned int id);
    ~Synchronization();

    ///////////// computation interface

    /**
     * Checks if threre are any incoming messages and then calls their handlers.
     */
    void synchronize();

    /**
     * Blocks until new work arrives or computation terminates.
     */
    bool isWorkAvailable();

    /**
     * Returns this instance's computation ID.
     */
    unsigned int getComputationID() { return computationID; }


    ///////////// network interface

    void informAssignment(disco_plat::blob data);
    void informNoAssignment();
    void informRequest(disco_plat::nodeID requesteeID);

    void informResult(unsigned int optimum, disco_plat::blob::_charDataSequence_seq data);

    void informTerminate();

    unsigned int getComputatuonID() { return computationID; }
    bool isWorking();
    bool hasWorkToSplit();

};

#endif // SYNCHRONIZATION_H
