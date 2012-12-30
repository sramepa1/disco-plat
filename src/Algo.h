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



// Abstract base class for algorithms carrying a problem instance with them.
// Extension point for supporting additional problems in disco-plat.

class AlgoInstance
{

public:

    AlgoInstance(Computation* comp) : comp(comp) {}

    virtual ~AlgoInstance() {}

    // Evaluate current state. Return true if it should be expanded, false if backtracking should occur.
    virtual bool evaluate() = 0;

    // Go deeper, expand the current state by pushing a new one.
    virtual void expand() = 0;

    // Print a text representation of the supplied configuration.
    virtual void printConfig(char* configuration, std::ostream& os) = 0;

    // Returns true if first opt_t operand is better than the second one;
    virtual bool isBetter(opt_t thisOptimum, opt_t thanThisOptimum) = 0;

protected:
    Computation* comp;
};

#endif // ALGORITHM_H
