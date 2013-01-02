#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "Algo.h"
#include "Computation.h"

class Synchronization;

#include <map>
#include <set>
#include <pthread.h>
#include <string>
#include <ostream>

#define INVALID_COMPUTATION_ID 0

class LeftNeighbourIface;
class RightNeighbourIface;


class Repository
{
public:
    Repository(std::string outputFileName);
    ~Repository();

    void init();


    ///////// Global resources

    uint64_t getTime() {
        pthread_mutex_lock(&timeMutex);
        uint64_t tmp = lamportTime;
        pthread_mutex_unlock(&timeMutex);
        return tmp;
    }

    std::ostream& getOutput();

    void lockCurrentSyncModule() { pthread_mutex_lock(&syncMutex); }
    Synchronization* getCurrentSyncModule() { return currentSyncModule; }
    void unlockCurrentSyncModule() { pthread_mutex_unlock(&syncMutex); }


    ///////// ID and computation data

    unsigned int getCurrentComputationID();
    unsigned int getFreeID();
    unsigned int getAnyValidID();

    void newData(unsigned int id, std::string algoName, std::string instanceText, bool broadcast);

    std::pair<AlgoInstance*, Computation*> getAlgoComp(unsigned int id);

    void startComputation(unsigned int id, bool localStart);

    // frees resources and marks ID as invalid
    void destroyComputation(unsigned int id);

    // destroys all computations not present in the given set (zombie-killing)
    void setSurvivingComputations(const std::set<unsigned int>& computationIDs);


    ///////// Node liveness and work cache

    bool isLiveNodeCacheConsistent();

    void setLiveNodeCacheConsistency(bool consistent);

    size_t getLiveNodeCount();

    // Always returns true if cache is marked as inconsistent
    bool isAlive(const std::string& identifier);

    std::set<std::string> getLiveNodes();

    void addLiveNode(const std::string &identifier);

    void setLiveNodes(const std::set<std::string>& liveNodes);


    ///////// interface for calling from networ init only

    void awakeInit();
    void awakeFreeID(unsigned int maxID);

    unsigned int getMaxID() { return maxID; }

    void sendAllData();
    void broadcastMyID();

    uint64_t timeIncrement() {
        pthread_mutex_lock(&timeMutex);
        uint64_t tmp = ++lamportTime;
        pthread_mutex_unlock(&timeMutex);
        return tmp;
    }

    void timeColision(uint64_t time) {
        pthread_mutex_lock(&timeMutex);
        lamportTime = (time > lamportTime ? time : lamportTime) + 1;
        pthread_mutex_unlock(&timeMutex);
    }


private:
    std::map<unsigned int, std::pair<std::string, std::string> > data;
    std::map<unsigned int, std::pair<AlgoInstance*, Computation*> > algoCompCache;

    std::set<std::string> liveNodes;

    pthread_mutex_t timeMutex;
    pthread_mutex_t dataMutex;
    pthread_mutex_t livenessMutex;
    pthread_mutex_t syncMutex;
    pthread_cond_t initCondition;

    bool isInitSleeping;
    bool isFreeIDSleeping;
    bool liveNodesConsistent;

    unsigned int currentID;
    unsigned int maxID;

    LeftNeighbourIface* leftNb;
    RightNeighbourIface* rightNb;

    Synchronization* currentSyncModule;

    std::ostream* outStream;
    bool isOutStreamOwner;

    uint64_t lamportTime;

    void waitThread();

    void destroyInternal(unsigned int computationID);

};

#endif // REPOSITORY_H
