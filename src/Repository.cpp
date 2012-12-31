
#include "globals.h"

#include "NeighbourIface.h"
#include "Repository.h"
#include "Synchronization.h"

#include <sstream>

using namespace std;
using namespace disco_plat;

Repository* repo;

Repository::Repository() {
    leftNb = &networkModule->getMyLeftInterface();
    rightNb = &networkModule->getMyRightInterface();

    pthread_mutex_init(&dataMutex, NULL);
    pthread_cond_init(&initCondition, NULL);

    isInitSleeping = false;
    isFreeIDSleeping = false;
}

Repository::~Repository() {
    pthread_mutex_destroy(&dataMutex);
    pthread_cond_destroy(&initCondition);
}

unsigned int Repository::getFreeID() {

    if(isSingleNode()) {
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

    pthread_mutex_unlock(&dataMutex);

    return it->first;
}

void Repository::newData(unsigned int id, std::string algoName, std::string instanceText, bool broadcast) {
    pthread_mutex_lock(&dataMutex);

    if(!data.insert(make_pair(id, make_pair(algoName, instanceText))).second)
    {
        pthread_mutex_unlock(&dataMutex);
        throw "Attempted to insert new data with an ID that already exists in this repository";
    }

    pthread_mutex_unlock(&dataMutex);

#ifdef VERBOSE
    cout << "Adding new instance data: " << instanceText;
#endif

    if(broadcast && !isSingleNode()) {

#ifdef VERBOSE
    cout << "Broadcasting new instance";
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

void Repository::destroy(unsigned int id) {
    pthread_mutex_lock(&dataMutex);

    data.erase(id);
    map<unsigned int, pair<AlgoInstance*, Computation*> >::iterator it = algoCompCache.find(id);
    if(it != algoCompCache.end()) {
        delete it->second.first;
        delete it->second.second;
        algoCompCache.erase(it);
    }

    pthread_mutex_unlock(&dataMutex);
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


void Repository::start(unsigned int id, bool localStart) {
    pair<AlgoInstance*, Computation*> algoComp = getAlgoComp(id);
    algoComp.second->start(localStart);
}


void Repository::init() {
    if(isSingleNode()) {
        maxID = 1;
        return;
    }

    pthread_mutex_lock(&dataMutex);
    isInitSleeping = true;

    pthread_cond_wait(&initCondition, &dataMutex);

    isInitSleeping = false;
    pthread_mutex_unlock(&dataMutex);
}


void Repository::awakeInit() {
    pthread_mutex_lock(&dataMutex);

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

    map<unsigned int, pair<string, string> >::iterator it;
    for(it = data.begin(); it != data.end(); ++it) {

        blob message;
        message.sourceNode = networkModule->getRightID(); // intentional hack to stop boomerang propagating
        message.computationID = it->first;
        message.messageType = INSTANCE_ANNOUNCEMENT;
        message.dataStringA = it->second.first.c_str();
        message.dataStringB = it->second.second.c_str();

        if(++it == data.end()) {
            message.slotA = BLOB_SA_IA_INIT_RESUME;
        } else {
            message.slotA = BLOB_SA_IA_NONE;
        }
        --it;

        rightNb->Boomerang(message);

    }

    pthread_mutex_unlock(&dataMutex);
}
