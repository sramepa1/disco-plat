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

    void startComputation(unsigned int id, bool localStart);

    // frees resources and marks ID as invalid
    void destroyComputation(unsigned int id);

    // destroys all computations not present in the given set (zombie-killing)
    void setSurvivingComputations(std::set<unsigned int> computationIDs);


    ///////// Node liveness and work cache

    bool isAlive(std::string& identifier);

    void addLiveNode(std::string& identifier);

    // Can be changed to CORBA sequence<string> if needed
    void setLiveNodes(std::set<std::string>& liveNodes);


    // TODO: Lamport timestamps?
    // Pass empty string as originalOwner if this is not a de-zombification update
    void updateWorkCache(std::string& identifier, WorkUnit& work, std::string& originalOwner);

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

    void destroyInternal(unsigned int computationID);
};

#endif // REPOSITORY_H
