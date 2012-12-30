#include "Algo.h"

using namespace std;

map<string, algoConstructor> AlgoFactory::constructors;

AlgoInstance* AlgoFactory::createAlgoInstance(string name, istream& textDataStream, Computation *comp) {
    map<string, algoConstructor>::iterator it = constructors.find(name);
    if(it == constructors.end()) {
        throw (string("No known constructor for algorithm named: ") + name).c_str();
    }
    return it->second(textDataStream, comp);
}

