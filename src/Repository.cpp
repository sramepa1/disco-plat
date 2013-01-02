
#include "globals.h"

#include "NeighbourIface.h"
#include "Repository.h"
#include "Synchronization.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <cstdlib>
#include <ctime>
extern "C" {
#include <unistd.h>
}


using namespace std;
using namespace disco_plat;

Repository* repo;

Repository::Repository(string outputFileName) : liveNodesConsistent(false), lamportTime(0) {
    leftNb = &networkModule->getMyLeftInterface();
    rightNb = &networkModule->getMyRightInterface();

    pthread_mutex_init(&timeMutex, NULL);
    pthread_mutex_init(&dataMutex, NULL);
    pthread_mutex_init(&livenessMutex, NULL);
    pthread_mutex_init(&syncMutex, NULL);
    pthread_cond_init(&initCondition, NULL);

    isInitSleeping = false;
    isFreeIDSleeping = false;

    if(outputFileName.empty()) {
        outStream = &cout;
        isOutStreamOwner = false;
    } else {
        outStream = new ofstream(outputFileName.c_str(), ios_base::app);
        isOutStreamOwner = true;
        if(!outStream->good()) {
            throw (string("Unable to open file \"") + outputFileName + "\" for output!").c_str();
        }
    }

    srand(time(NULL) ^ getpid());
}

Repository::~Repository() {
    pthread_mutex_destroy(&timeMutex);
    pthread_mutex_destroy(&dataMutex);
    pthread_mutex_destroy(&livenessMutex);
    pthread_mutex_destroy(&syncMutex);
    pthread_cond_destroy(&initCondition);

    if(isOutStreamOwner) {
        delete outStream;
    }
}

ostream& Repository::getOutput() {
    pthread_mutex_lock(&timeMutex);
    *outStream << '[' << hex << setw(16) << setfill('0') << uppercase << lamportTime << "] " << dec;
    pthread_mutex_unlock(&timeMutex);
    return *outStream;
}

unsigned int Repository::getCurrentComputationID() {
    lockCurrentSyncModule();
    unsigned int value = currentID;
    unlockCurrentSyncModule();
    return value;
}

unsigned int Repository::getFreeID() {

    if(networkModule->isSingle()) {
        return maxID++;
    }


    blob message;
    message.sourceNode = networkModule->getMyID();
    message.computationID = BLOB_CID_GLOBAL_ID;
    message.messageType = FREE_ID_SEARCH;
    message.slotA = maxID;

    rightNb->Boomerang(message);


    pthread_mutex_lock(&dataMutex);
    isFreeIDSleeping = true;

    pthread_cond_wait(&initCondition, &dataMutex);

    isFreeIDSleeping = false;
    pthread_mutex_unlock(&dataMutex);


    return maxID;
}

unsigned int Repository::getAnyValidID() {
    pthread_mutex_lock(&dataMutex);

    map<unsigned int, pair<string, string> >::iterator it = data.begin();
    if(it == data.end())
    {
        pthread_mutex_unlock(&dataMutex);
        return INVALID_COMPUTATION_ID;
    }

    // Randomize which computation to join
    int shift = rand() % data.size();
    for(int i = 0; i < shift; i++) {
        ++it;
    }

    pthread_mutex_unlock(&dataMutex);

    return it->first;
}

void Repository::newData(unsigned int id, string algoName, string instanceText, bool broadcast) {

#ifdef VERBOSE
    getOutput() << "Adding new instance data: " << instanceText;
#endif

    pthread_mutex_lock(&dataMutex);
    if(!data.insert(make_pair(id, make_pair(algoName, instanceText))).second) {
        #ifdef VERBOSE
        getOutput() << "Warning: instance data with ID " << id
                    << " overwritten! Let's hope it was just a net-crash repeat...";
        #endif
    }
    pthread_mutex_unlock(&dataMutex);

    if(broadcast && !networkModule->isSingle()) {

#ifdef VERBOSE
    getOutput() << "Broadcasting new instance";
#endif

        // let the ring know
        blob message;
        message.sourceNode = networkModule->getMyID();
        message.computationID = id;
        message.messageType = INSTANCE_ANNOUNCEMENT;
        message.dataStringA = algoName.c_str();
        message.dataStringB = instanceText.c_str();
        message.slotA = BLOB_SA_IA_NONE;

        rightNb->Boomerang(message);
    }
}

void Repository::destroyComputation(unsigned int id) {
    pthread_mutex_lock(&dataMutex);
    destroyInternal(id);
    pthread_mutex_unlock(&dataMutex);
}


void Repository::setSurvivingComputations(const set<unsigned int>& computationIDs) {
    pthread_mutex_lock(&dataMutex);

    map<unsigned int, pair<string, string> >::iterator it = data.begin();
    while(it != data.end()) {
        if(computationIDs.find(it->first) == computationIDs.end()) {
            destroyInternal((it++)->first); // post-increment required as the iterator will be invalidated!
        } else {
            ++it;
        }
    }

    pthread_mutex_unlock(&dataMutex);
}


void Repository::destroyInternal(unsigned int computationID) {
    data.erase(computationID);
    map<unsigned int, pair<AlgoInstance*, Computation*> >::iterator it = algoCompCache.find(computationID);
    if(it != algoCompCache.end()) {
        delete it->second.first;
        delete it->second.second;
        algoCompCache.erase(it);
    }
}


pair<AlgoInstance*, Computation*> Repository::getAlgoComp(unsigned int id) {
    map<unsigned int, pair<AlgoInstance*, Computation*> >::iterator it = algoCompCache.find(id);
    if(it == algoCompCache.end())
    {
        pthread_mutex_lock(&dataMutex);

        map<unsigned int, pair<string, string> >::iterator dit = data.find(id);
        if(dit == data.end())
        {
            pthread_mutex_unlock(&dataMutex);
            throw "Requested an algo-computation pair for ID not matching any data in this repository!";
        }

        pthread_mutex_unlock(&dataMutex);

        Computation* comp = new Computation();
        istringstream textDataStream(dit->second.second);
        AlgoInstance* algo = AlgoFactory::createAlgoInstance(dit->second.first, textDataStream, comp);
        comp->setAlgorithm(algo);
        comp->setSync(new Synchronization(comp, id));

        pair<AlgoInstance*, Computation*> result = make_pair(algo, comp);
        algoCompCache[id] = result;
        return result;
    }
    return it->second;
}


void Repository::startComputation(unsigned int id, bool localStart) {

    pair<AlgoInstance*, Computation*> algoComp = getAlgoComp(id);

    lockCurrentSyncModule();
    currentSyncModule = algoComp.second->getSync();
    currentID = id;
    unlockCurrentSyncModule();

    algoComp.second->start(localStart);

    lockCurrentSyncModule();
    currentSyncModule = NULL;
    currentID = INVALID_COMPUTATION_ID;
    unlockCurrentSyncModule();
}


void Repository::init() {

    if(networkModule->isSingle()) {
#ifdef VERBOSE
    getOutput() << "I am a single node - no data gathering needed" << endl;
#endif
        maxID = 1;

        pthread_mutex_lock(&livenessMutex);
        liveNodes.insert(string(networkModule->getMyID().identifier));
        pthread_mutex_unlock(&livenessMutex);

        return;
    }

#ifdef VERBOSE
    getOutput() << "Starting data gathering - idling main thread" << endl;
#endif

    pthread_mutex_lock(&dataMutex);
    isInitSleeping = true;

    pthread_cond_wait(&initCondition, &dataMutex);

    isInitSleeping = false;
    pthread_mutex_unlock(&dataMutex);
}


void Repository::awakeInit() {
    pthread_mutex_lock(&dataMutex);

#ifdef VERBOSE
    getOutput() << "Initial data gathering complete - waking main thread" << endl;
#endif

    if(isInitSleeping) {
        pthread_cond_signal(&initCondition);
    }

    pthread_mutex_unlock(&dataMutex);
}


void Repository::awakeFreeID(unsigned int maxID) {
    pthread_mutex_lock(&dataMutex);

    if(isFreeIDSleeping) {
        this->maxID = maxID + 1;
        pthread_cond_signal(&initCondition);
    }

    pthread_mutex_unlock(&dataMutex);
}

void Repository::sendAllData() {
    pthread_mutex_lock(&dataMutex);

    // send live nodes
    set<string>::iterator lit = liveNodes.begin();
    while(lit != liveNodes.end()) {
        blob message;
        message.sourceNode = networkModule->getRightID(); // intentional hack to stop boomerang propagating
        message.computationID = 0;
        message.messageType = NODE_ANNOUNCEMENT;
        message.dataStringA = (*lit).c_str();

        if(++lit == liveNodes.end()) {
            message.slotA = 1;
        } else {
            message.dataStringB= (*lit).c_str();
            message.slotA = 2;
            ++lit;
        }

        rightNb->Boomerang(message);

    }

    // send instances
    map<unsigned int, pair<string, string> >::iterator dit;
    for(dit = data.begin(); dit != data.end(); ++dit) {

        blob message;
        message.sourceNode = networkModule->getRightID(); // intentional hack to stop boomerang propagating
        message.computationID = dit->first;
        message.messageType = INSTANCE_ANNOUNCEMENT;
        message.dataStringA = dit->second.first.c_str();
        message.dataStringB = dit->second.second.c_str();

        if(++dit == data.end()) {
            message.slotA = BLOB_SA_IA_INIT_RESUME;
        } else {
            message.slotA = BLOB_SA_IA_NONE;
        }
        --dit;

        rightNb->Boomerang(message);

    }

    pthread_mutex_unlock(&dataMutex);
}

void Repository::broadcastMyID() {
    blob message;
    message.sourceNode = networkModule->getMyID();
    message.computationID = 0;
    message.messageType = NODE_ANNOUNCEMENT;
    message.dataStringA = networkModule->getMyID().identifier;

    rightNb->Boomerang(message);
}

bool Repository::isLiveNodeCacheConsistent() {
    pthread_mutex_lock(&livenessMutex);
    bool result = liveNodesConsistent;
    pthread_mutex_unlock(&livenessMutex);
    return result;
}

void Repository::setLiveNodeCacheConsistency(bool consistent) {
    pthread_mutex_lock(&livenessMutex);
    liveNodesConsistent = consistent;
    pthread_mutex_unlock(&livenessMutex);
}

size_t Repository::getLiveNodeCount() {
    pthread_mutex_lock(&livenessMutex);
    size_t tmp = liveNodes.size();
    pthread_mutex_unlock(&livenessMutex);
    return tmp;
}

bool Repository::isAlive(const string& identifier) {
    pthread_mutex_lock(&livenessMutex);
    bool result = !liveNodesConsistent || liveNodes.find(identifier) != liveNodes.end();
    pthread_mutex_unlock(&livenessMutex);
    return result;
}

set<string> Repository::getLiveNodes() {
    pthread_mutex_lock(&livenessMutex);
    set<string> tmp = liveNodes;
    pthread_mutex_unlock(&livenessMutex);
    return tmp;
}

void Repository::addLiveNode(const string& identifier) {
    pthread_mutex_lock(&livenessMutex);
#ifdef VERBOSE
    getOutput() << "Adding new living node: " << identifier << endl;
#endif
    liveNodes.insert(identifier);
    pthread_mutex_unlock(&livenessMutex);
}

void Repository::setLiveNodes(const set<string>& liveNodes) {
    pthread_mutex_lock(&livenessMutex);

    set<string> zombies;
    set_difference(this->liveNodes.begin(), this->liveNodes.end(),
                   liveNodes.begin(), liveNodes.end(),
                    inserter(zombies, zombies.end()));

    this->liveNodes = liveNodes;

    for(map<unsigned int, pair<AlgoInstance*, Computation*> >::iterator it = algoCompCache.begin();
                                                                        it != algoCompCache.end(); ++it) {
        Computation* comp = it->second.second;
        comp->getSync()->zombify(zombies);
    }

    pthread_mutex_unlock(&livenessMutex);
}
