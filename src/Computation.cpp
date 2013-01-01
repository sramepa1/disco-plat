#include "Computation.h"
#include "Synchronization.h"
#include "Algo.h"

extern "C" {
#include <string.h>
#include <unistd.h>
#include <time.h>
}

using namespace std;

Computation::Computation() :    algo(NULL), sync(NULL),
                                configStack(NULL), configStackCopy(NULL),
                                intervalStack(NULL), intervalStackCopy(NULL),
                                stackTop(0),
                                optimum(0), optimalConfig(NULL),
                                newSolutionFound(false), absoluteSolution(false),
                                loopCounter(0), loopsToSync(3000000)
{
    timestamp = areWeThereYet();
}

Computation::~Computation() {
    deallocateAll();
}

uint64_t Computation::areWeThereYet() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    uint64_t now = ts.tv_sec * (uint64_t)1000 + ts.tv_nsec / (uint64_t)1000000; // get milliseconds
    return now;
}


void Computation::setAlgorithm(AlgoInstance *algo) {
    this->algo = algo;
}


void Computation::setSync(Synchronization* sync) {
    this->sync = sync;
}


void Computation::start(bool localStart) {
    currentSyncModule = sync;

    cout << (localStart ? "Initiating" : "Joining") << " computation " << sync->getComputationID() << endl;

    if(!localStart && !sync->isWorkAvailable()) {
        cout << "Late to the party... Joined a computation where noone wants to share work. Quitting." << endl;
        currentSyncModule = NULL;
        return;
    }

    try {

        do {
            DFS();
        } while(sync->isWorkAvailable());

    } catch(AbsoluteSolutionException) {
        cout << "Absolute solution detected." << endl;
    }

    cout << "Computation " << sync->getComputationID() << " finished with the following result:" << endl;
    algo->printConfig(optimalConfig, cout);

    currentSyncModule = NULL;
}

void Computation::DFS() {

    loopCounter = 0;

    while(true) {

//      cout << "Trying configuration => ";
//      algo->printConfig(configStack + stackTop*instanceSize, cout);

        if(algo->evaluate()) {
            algo->expand();

        } else {
            // backtrack
            if(--stackTop < 0) {
                break;
            }
        }

        if(++loopCounter >= loopsToSync) {
            synchronize();
            loopCounter = 0;
        }
    }
}

void Computation::synchronize() {

    uint64_t now = areWeThereYet();
    uint64_t delta = now - timestamp;
    timestamp = now;
    if(delta < 300) {
        #ifdef VERBOSE
        cout << "Adaptive loop delta was too low: " << delta << ", doubling loop counter" << endl;
        #endif
        loopsToSync *= 2;
    }
    if(delta > 1000) {
        #ifdef VERBOSE
        cout << "Adaptive loop delta was too high: " << delta << ", halving loop counter" << endl;
        #endif
        loopsToSync /= 2;
    }

    sync->synchronize();

    if(absoluteSolution) {
        throw AbsoluteSolutionException();
    }
}


pair<opt_t, vector<char> > Computation::getSolution() {
    vector<char> vec(optimalConfig, optimalConfig + instanceSize);
    newSolutionFound = false;
    return make_pair(optimum, vec);
}


bool Computation::isBetter(opt_t thisOptimum, opt_t thanThisOptimum) {
    return algo->isBetter(thisOptimum, thanThisOptimum);
}


bool Computation::splitWork(WorkUnit& work) {

    //int depth = stackTop + 1;
    //work.depth = depth;

    //work.configStackVector.clear();
    //work.configStackVector.resize(instanceSize * depth);

    return false; // TODO: Actual work splitting
}


void Computation::setWork(WorkUnit& work) {

    if(instanceSize != work.instanceSize) {
        throw "Attempted to assign an incompatible work unit!";
    }

    stackTop = work.depth - 1;
    memcpy(configStack, work.configStackVector.data(), instanceSize * work.depth);

    int i = 0;
    for(vector<int>::iterator it = work.intervalStackVector.begin(); it != work.intervalStackVector.end(); ++it) {
        int tmp = *it++;
        intervalStack[i++] = pair<int, int>(tmp, *it);
    }

    algo->dataChanged();
    newSolutionFound = false;
}


void Computation::setSolution(opt_t optimum, vector<char> configuration, bool isAbsolute) {
    this->optimum = optimum;
    memcpy(optimalConfig, configuration.data(), instanceSize);
    absoluteSolution = isAbsolute;
}


void Computation::reinitialize(int instanceSize, opt_t initialOptimum, char* initialConfiguration) {
    this->instanceSize = instanceSize;
    optimum = initialOptimum;
    stackTop = 0;
    absoluteSolution = false;
    newSolutionFound = false;

    deallocateAll();

    configStack = new char[instanceSize * (instanceSize + 1)];
    intervalStack = new pair<int, int>[instanceSize * (instanceSize + 1)];
    optimalConfig = new char[instanceSize];

    memcpy(optimalConfig, initialConfiguration, instanceSize);
    memcpy(configStack, initialConfiguration, instanceSize);
    intervalStack[0] = pair<int, int>(0, instanceSize);
}

void Computation::newSolution(opt_t optimum, char* configuration, bool isAbsolute) {
    this->optimum = optimum;
    memcpy(optimalConfig, configuration, instanceSize);
    if(isAbsolute) {
        absoluteSolution = true;
        loopCounter = loopsToSync; // sync immediately
    }
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
