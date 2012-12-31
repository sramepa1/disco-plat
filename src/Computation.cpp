#include "Computation.h"
#include "Synchronization.h"
#include "Algo.h"

extern "C" {
#include <string.h>
#include <unistd.h>
}

using namespace std;

Computation::Computation() :    algo(NULL), sync(NULL),
                                configStack(NULL), configStackCopy(NULL),
                                intervalStack(NULL), intervalStackCopy(NULL),
                                stackTop(0),
                                optimum(0), optimalConfig(NULL),
                                newSolutionFound(false), trivialSolution(false),
                                loopsToSync(1000)   // TODO: Increase to 100K ~ 1M and make adaptive
{
}

Computation::~Computation() {
    deallocateAll();
}


/*
 *  WORK IN PROGRESS, THIS WILL LIKELY CHANGE A LOT!
 */


void Computation::setAlgorithm(AlgoInstance *algo) {
    this->algo = algo;
}


void Computation::setSync(Synchronization* sync) {
    this->sync = sync;
}



void Computation::start() {
    currentSyncModule = sync;   // TODO: Mutex

    do {
        DFS();
    } while(sync->isWorkAvailable());

    cout << "Computation finished with the following result:" << endl;
    algo->printConfig(optimalConfig, cout);
}

void Computation::DFS() {
    try {
        int synchroCounter = 0;

        while(true) {

//            cout << "Trying configuration => ";
//            algo->printConfig(configStack + stackTop*instanceSize, cout);

            if(algo->evaluate()) {
                algo->expand();

            } else {
                // backtrack
                if(--stackTop < 0) {
                    break;
                }
            }

            if(++synchroCounter >= loopsToSync) {
                synchronize();
                synchroCounter = 0;
            }
        }


    } catch(TrivialSolutionException) {
        cout << "Remote trivial solution detected" << endl;
    }
}

void Computation::synchronize() {

    // TODO: Adaptively update loopsToSync

    sync->synchronize();
    if(trivialSolution) {
        throw TrivialSolutionException();
    }
}


pair<opt_t, vector<char> > Computation::getSolution() {
    vector<char> vec(optimalConfig, optimalConfig + instanceSize);
    return make_pair(optimum, vec);
}


bool Computation::isBetter(opt_t thisOptimum, opt_t thanThisOptimum) {
    return algo->isBetter(thisOptimum, thanThisOptimum);
}


vector<char*> Computation::splitWork(int requestCount) {
    return vector<char*>(); // TODO: Actual work splitting
}


void Computation::setWork(const char* configStack, const std::pair<int,int> * intervalStack, int depth) {
    /*TODO: implement*/
    algo->dataChanged();
    newSolutionFound = false;
}


void Computation::setSolution(opt_t optimum, vector<char> configuration, bool isTrivial) {
    this->optimum = optimum;
    memcpy(optimalConfig, configuration.data(), instanceSize);
    trivialSolution = isTrivial;
}


void Computation::reinitialize(int instanceSize, opt_t initialOptimum, char* initialConfiguration) {
    this->instanceSize = instanceSize;
    optimum = initialOptimum;
    stackTop = 0;
    trivialSolution = false;
    newSolutionFound = false;

    deallocateAll();

    configStack = new char[instanceSize * instanceSize];
    intervalStack = new pair<int, int>[instanceSize * instanceSize];
    optimalConfig = new char[instanceSize];

    memcpy(optimalConfig, initialConfiguration, instanceSize);
    memcpy(configStack, initialConfiguration, instanceSize);
    intervalStack[0] = pair<int, int>(0, instanceSize);
}

void Computation::newSolution(opt_t optimum, char* configuration) {
    this->optimum = optimum;
    memcpy(optimalConfig, configuration, instanceSize);
    newSolutionFound = true;
}

/**
 * Changes the "next" node to examine at the top of the stack upon backtrack to current state again.
 */
void Computation::setInterval(std::pair<int,int> interval) {
    intervalStack[stackTop] = interval;
}

void Computation::peekState(char* & configuration, std::pair<int,int> & interval) {
    configuration = configStack + stackTop * instanceSize;
    interval = intervalStack[stackTop];
}

void Computation::popState(char* & configuration, std::pair<int,int> & interval) {
    peekState(configuration, interval);
    stackTop--;
}

void Computation::pushState(char* configuration, std::pair<int,int> interval) {
    stackTop++;
    memcpy(configStack + stackTop * instanceSize, configuration, instanceSize);
    intervalStack[stackTop] = interval;
}

void Computation::deallocateAll() {
    delete[] configStack;
    delete[] configStackCopy;
    delete[] intervalStack;
    delete[] intervalStackCopy;
    delete[] optimalConfig;
}
