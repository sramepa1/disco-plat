#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include "Computation.h"

typedef AlgoInstance* (*algoConstructor) (std::istream&, Computation*);

class AlgoFactory
{
public:
    static AlgoInstance* createAlgoInstance(std::string name, std::istream& textDataStream, Computation* comp);

    static  void registerAlgorithm(std::string name, algoConstructor constructor) {
        constructors.insert(make_pair(name, constructor));
    }
private:
    AlgoFactory() {}
    AlgoFactory(const AlgoFactory& other) {}
    AlgoFactory& operator=(const AlgoFactory& other) {return *this;}

    static std::map<std::string, algoConstructor> constructors;
};


// Abstract base class for algorithms carrying a problem instance
class AlgoInstance
{

public:

    AlgoInstance(Computation* comp) : comp(comp) {}

    virtual ~AlgoInstance() {}

    virtual void expand() = 0;

    virtual void printConfig(char* configuration, std::ostream& os) = 0;

protected:
    Computation* comp;
};

#endif // ALGORITHM_H
