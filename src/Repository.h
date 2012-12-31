#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "Algo.h"
#include "Computation.h"

#include <map>
#include <pthread.h>
#include <string>

#define INVALID_COMPUTATION_ID 0

class Repository
{
public:
    Repository();
    ~Repository();

    ///////// general interface

    unsigned int getFreeID();
    unsigned int getAnyValidID();

    void newData(unsigned int id, std::string algoName, std::string instanceText, bool broadcast);

    std::pair<AlgoInstance*, Computation*> getAlgoComp(unsigned int id);

    void init();

    void start(unsigned int id);

    // frees resources and marks ID as invalid
    void destroy(unsigned int id);


    ///////// interface for calling from networ init only

    void awakeInit();
    void awakeFreeID(unsigned int maxID);

    unsigned int getMaxID() { return maxID; }

    void sendAllData();

private:
    std::map<unsigned int, std::pair<std::string, std::string> > data;
    std::map<unsigned int, std::pair<AlgoInstance*, Computation*> > algoCompCache;

    pthread_mutex_t dataMutex;
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
