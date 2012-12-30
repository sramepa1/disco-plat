#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "Algo.h"
#include "Computation.h"

#include <string>
#include <map>

class Repository
{
public:
    Repository();

    unsigned int getFreeID();
    unsigned int getAnyValidID();

    void newData(unsigned int id, std::string algoName, std::string instanceText, bool broadcast);

    std::pair<AlgoInstance*, Computation*> getAlgoComp(unsigned int id);

    void start(unsigned int id);

    // frees resources and marks ID as invalid
    void destroy(unsigned int id);

private:
    std::map<unsigned int, std::pair<std::string, std::string> > data;
    std::map<unsigned int, std::pair<AlgoInstance*, Computation*> > algoCompCache;
};

#endif // REPOSITORY_H
