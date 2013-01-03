#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include "globals.h"
#include "NeighbourIface.h"
#include "Computation.h"

#include <list>
#include <pthread.h>
#include <vector>
#include <map>
#include <set>
#include <string>


#define BLACK false
#define WHITE true

#define PING_COUNTER_MAX 15

#define SOLUTION_ABSOLUTE 1
#define SOLUTION_TERMINAL 2

enum SyncState { WORKING, SYNCHRONIZING, IDLING, TERMINATING };


class Synchronization
{
    Computation* comp;

    LeftNeighbourIface* leftNb;
    RightNeighbourIface* rightNb;

    unsigned int computationID;

    std::vector<char> circleConfiguration;
    opt_t circleSolutionOpt;
    bool circleSolutionAbsolute;
    bool haveNewCirlceSolution;

    std::vector<char> myConfiguration;
    opt_t mySolutionOpt;
    bool mySolutionAbsolute;
    bool haveMySolution;

    struct WorkUnit tmpUnit;
    struct WorkUnit unitToAnnounceKept;

    bool isWorkingState;
    bool workReciewed;
    bool workRequestPending;
    bool isZombifyingState;
    bool splitSuccesful;

    int pingCounter;

    std::list<disco_plat::nodeID> workRequests;
    std::map<std::string, std::pair<uint64_t, WorkUnit> > workAssignments;
    std::map<std::string, std::pair<uint64_t, WorkUnit> > zombieWork;

    typedef bool color;
    color myColor;

    bool terminationToken;
    disco_plat::nodeID tokenOrigin;
    color recievedColor;

    bool terminating;
    bool terminationLeader;
    bool commandTerminate;

    pthread_mutex_t stateMutex;
    pthread_mutex_t syncMutex;
    pthread_mutex_t workCacheMutex;
    pthread_cond_t idleCondition;
    pthread_cond_t terminalCondition;


    void sendTerminationToken();
    void sendWorkRequest();


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

    void updateWorkCache(disco_plat::blob& assignmentData);

    void zombify(const std::set<std::string>& deadIdentifiers);

    // Hard erase without moving. Only use when the cached work can't be lost.
    void killZombie(const std::string& zombieIdentifier);

    void informResult(disco_plat::blob& data);

    void informMyToken(disco_plat::blob data);
    void informForeignToken(disco_plat::blob data);

    void informTerminate(disco_plat::blob data);

    void informNetworkRebuild();
    void informZombifyFinish();

    void pingReset() {
        pthread_mutex_lock(&syncMutex);
        pingCounter = 0;
        pthread_mutex_unlock(&syncMutex);
    }

    bool isZombifying();
    bool isWorking();
    bool hasWorkToSplit();

};

#endif // SYNCHRONIZATION_H
