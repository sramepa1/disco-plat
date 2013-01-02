#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include "globals.h"
#include "NeighbourIface.h"
#include "Computation.h"

#include <list>
#include <pthread.h>
#include <vector>
#include <map>
#include <string>


#define BLACK false
#define WHITE true

#define PING_COUNTER_MAX 15

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

    struct WorkUnit unit;

    bool isWorkingState;
    bool workReciewed;
    bool splitSuccesful;

    int pingCounter;

    std::list<disco_plat::nodeID> workRequests;
    std::map<std::string, std::pair<uint64_t, WorkUnit> > workAssignments;

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
    pthread_cond_t idleCondition;


    void sendTerminationToken();


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

    // Pass empty string as originalOwner if this is not a de-zombification update
    void updateWorkCache(std::string& identifier, uint64_t time, WorkUnit& work, std::string& originalOwner);

    void informResult(disco_plat::blob data);

    void informMyToken(disco_plat::blob data);
    void informForeignToken(disco_plat::blob data);

    void informTerminate(disco_plat::blob data);

    void pingReset() {
        pthread_mutex_lock(&syncMutex);
        pingCounter = 0;
        pthread_mutex_unlock(&syncMutex);
    }

    bool isWorking();
    bool hasWorkToSplit();

};

#endif // SYNCHRONIZATION_H
