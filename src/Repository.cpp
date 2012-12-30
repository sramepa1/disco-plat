#include "Repository.h"
#include "Synchronization.h"

#include <sstream>

using namespace std;

Repository* repo;

Repository::Repository()
{
    // TODO: Populate this->data from network!
}

unsigned int Repository::getFreeID() {
    return 42;  // TODO: Return a free computation ID
}

unsigned int Repository::getAnyValidID() {
    map<unsigned int, pair<string, string> >::iterator it = data.begin();
    if(it == data.end()) {
        throw "Requested a valid computation ID from an empty repository!";
    }
    return it->first;
}

void Repository::newData(unsigned int id, std::string algoName, std::string instanceText, bool broadcast) {
    if(!data.insert(make_pair(id, make_pair(algoName, instanceText))).second) {
        throw "Attempted to insert new data with an ID that already exists in this repository";
    }

    if(broadcast) {
        // TODO: Let the ring know!
    }
}

void Repository::destroy(unsigned int id) {
    data.erase(id);
    map<unsigned int, pair<AlgoInstance*, Computation*> >::iterator it = algoCompCache.find(id);
    if(it != algoCompCache.end()) {
        delete it->second.first;
        delete it->second.second;
        algoCompCache.erase(it);
    }
}

pair<AlgoInstance*, Computation*> Repository::getAlgoComp(unsigned int id) {
    map<unsigned int, pair<AlgoInstance*, Computation*> >::iterator it = algoCompCache.find(id);
    if(it == algoCompCache.end()) {
        map<unsigned int, pair<string, string> >::iterator dit = data.find(id);
        if(dit == data.end()) {
            throw "Requested a algo-computation pair for ID not matching any data in this repository!";
        }
        Computation* comp = new Computation();
        istringstream textDataStream(dit->second.second);
        AlgoInstance* algo = AlgoFactory::createAlgoInstance(dit->second.first, textDataStream, comp);
        comp->setAlgorithm(algo);
        comp->setSync(new Synchronization(comp));

        pair<AlgoInstance*, Computation*> result = make_pair(algo, comp);
        algoCompCache[id] = result;
        return result;
    }
    return it->second;
}

void Repository::start(unsigned int id) {
    pair<AlgoInstance*, Computation*> algoComp = getAlgoComp(id);
    algoComp.second->start();
}
