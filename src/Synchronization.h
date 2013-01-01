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

    std::list<disco_plat::nodeID> workRequests;
    std::map<std::string, WorkUnit> workAssignments;

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

    // TODO: Lamport timestamps?
    // Pass empty string as originalOwner if this is not a de-zombification update
    void updateWorkCache(std::string& identifier, WorkUnit& work, std::string& originalOwner);

    void informResult(disco_plat::blob data);

    void informMyToken(disco_plat::blob data);
    void informForeignToken(disco_plat::blob data);

    void informTerminate();

    bool isWorking();
    bool hasWorkToSplit();

};

#endif // SYNCHRONIZATION_H
