#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "Algo.h"
#include "Computation.h"
#include "Interface.h"

#include <map>
#include <set>
#include <pthread.h>
#include <string>

#define INVALID_COMPUTATION_ID 0

class Repository
{
public:
    Repository();
    ~Repository();

    void init();

    ///////// ID and computation data

    unsigned int getFreeID();
    unsigned int getAnyValidID();

    void newData(unsigned int id, std::string algoName, std::string instanceText, bool broadcast);

    std::pair<AlgoInstance*, Computation*> getAlgoComp(unsigned int id);

    void start(unsigned int id, bool localStart);

    // frees resources and marks ID as invalid
    void destroy(unsigned int id);


    ///////// Node liveness and work cache

    bool isAlive(CORBA::String_var& identifier);

    void addLiveNode(CORBA::String_var& identifier);

    // Can be changed to CORBA sequence<string> if needed
    void setLiveNodes(std::set<std::string>& liveNodes);


    // TODO: Lamport timestamps?
    // Pass empty string as originalOwner if this is not a de-zombification update
    void updateWorkCache(CORBA::String_var& identifier, WorkUnit& work, CORBA::String_var& originalOwner);

    ///////// interface for calling from networ init only

    void awakeInit();
    void awakeFreeID(unsigned int maxID);

    unsigned int getMaxID() { return maxID; }

    void sendAllData();

private:
    std::map<unsigned int, std::pair<std::string, std::string> > data;
    std::map<unsigned int, std::pair<AlgoInstance*, Computation*> > algoCompCache;

    std::set<std::string> liveNodes;
    std::map<std::string, WorkUnit> assignedWork;

    pthread_mutex_t dataMutex;
    pthread_mutex_t livenessMutex;
    pthread_cond_t initCondition;

    bool isInitSleeping;
    bool isFreeIDSleeping;

    unsigned int maxID;

    LeftNeighbourIface* leftNb;
    RightNeighbourIface* rightNb;

    void waitThread();

    bool isSingleNode() { return networkModule->getLeftID().identifier == networkModule->getMyID().identifier; }
};

#endif // REPOSITORY_H
