#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "Algo.h"
#include "Computation.h"
#include "Interface.h"

#include <map>
#include <set>
#include <pthread.h>
#include <string>
#include <ostream>

#define INVALID_COMPUTATION_ID 0

class Repository
{
public:
    Repository(std::string outputFileName);
    ~Repository();

    void init();

    std::ostream& getOutput() { return *outStream; }

    ///////// ID and computation data

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

    size_t getLiveNodeCount() { return liveNodes.size(); }

    bool isAlive(const std::string& identifier);

    void addLiveNode(const std::string &identifier);

    // Can be changed to CORBA sequence<string> if needed
    void setLiveNodes(const std::set<std::string>& liveNodes);



    ///////// interface for calling from networ init only

    void awakeInit();
    void awakeFreeID(unsigned int maxID);

    unsigned int getMaxID() { return maxID; }

    void sendAllData();
    void broadcastMyID();

private:
    std::map<unsigned int, std::pair<std::string, std::string> > data;
    std::map<unsigned int, std::pair<AlgoInstance*, Computation*> > algoCompCache;

    std::set<std::string> liveNodes;

    pthread_mutex_t dataMutex;
    pthread_mutex_t livenessMutex;
    pthread_cond_t initCondition;

    bool isInitSleeping;
    bool isFreeIDSleeping;

    unsigned int maxID;

    LeftNeighbourIface* leftNb;
    RightNeighbourIface* rightNb;

    std::ostream* outStream;
    bool isOutStreamOwner;

    void waitThread();

    bool isSingleNode() { return networkModule->getLeftID().identifier == networkModule->getMyID().identifier; }

    void destroyInternal(unsigned int computationID);
};

#endif // REPOSITORY_H
