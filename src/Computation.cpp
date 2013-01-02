#include "Computation.h"
#include "Synchronization.h"
#include "Algo.h"
#include "globals.h"
#include "Repository.h"

extern "C" {
#include <string.h>
#include <unistd.h>
#include <time.h>
}

using namespace std;

Computation::Computation() :    algo(NULL), sync(NULL),
                                configStack(NULL),
                                intervalStack(NULL),
                                stackTop(0), maxStackSize(0),
                                optimum(0), optimalConfig(NULL),
                                newSolutionFound(false), absoluteSolution(false), workSplitPossible(true),
                                loopCounter(0), loopsToSync(500000)
{
    timestamp = areWeThereYet();
}

Computation::~Computation() {
    deallocateAll();
    delete sync;
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
    repo->getOutput() << (localStart ? "Initiating" : "Joining") << " computation " << sync->getComputationID() << endl;

    if(!localStart && !sync->isWorkAvailable()) {
        repo->getOutput() << "Late to the party... Joined a computation where noone wants to share work. Quitting." << endl;
        return;
    }

    try {

        do {
            DFS();
        } while(sync->isWorkAvailable());

    } catch(AbsoluteSolutionException) {
        repo->getOutput() << "Absolute solution detected." << endl;
    }

    repo->getOutput() << "Computation " << sync->getComputationID() << " finished with the following result:" << endl;
    algo->printConfig(optimalConfig, repo->getOutput());
}

void Computation::DFS() {

    loopCounter = 0;

    while(true) {

//      repo->getOutput() << "Trying configuration => ";
//      algo->printConfig(configStack + stackTop*instanceSize, repo->getOutput());

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
    if(delta < 50) {
        #ifdef VERBOSE
        repo->getOutput() << "Adaptive loop delta was too low: " << delta << ", increasing loop counter" << endl;
        #endif
        loopsToSync *= 3;
        loopsToSync /= 2;
    }
    if(delta > 300) {
        #ifdef VERBOSE
        repo->getOutput() << "Adaptive loop delta was too high: " << delta << ", decreasing loop counter" << endl;
        #endif
        loopsToSync /= 3;
        loopsToSync *= 2;
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

    if(!workSplitPossible) {
        return false;
    }

    work.instanceSize = instanceSize;
    work.intervalStackVector.clear();

    // avoid splitting on very deep levels (above 60% of theoretical maximum)
    for(int stackLevel = 0; stackLevel <= stackTop && stackLevel <= (maxStackSize * 3) / 5; stackLevel++) {

        work.configStackVector.resize(instanceSize * (stackLevel + 1));
        memcpy(work.configStackVector.data() + stackLevel * instanceSize,
               configStack + stackLevel * instanceSize,
               instanceSize);

        int left = intervalStack[stackLevel].first;
        int right = intervalStack[stackLevel].second;
        int branchCount = right - left;

        if(branchCount >= 2) {
            int amountToGive = (int)((double)branchCount * 0.9); // Bulgarian constant rulez
            int cut = right - amountToGive;

            work.intervalStackVector.push_back(cut);
            work.intervalStackVector.push_back(right);
            intervalStack[stackLevel].second = cut;

            #ifdef VERBOSE
            repo->getOutput() << "Keeping work slice [" << left << ", " << cut << ") at level " << stackLevel << endl;
            #endif

            work.depth = stackLevel + 1;
            return true;
        }

        work.intervalStackVector.push_back(left);
        work.intervalStackVector.push_back(right);
    }

    return (workSplitPossible = false); // TODO: Discuss optimality of outcome caching and point-of-no-return behavior
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

    #ifdef VERBOSE
    repo->getOutput() << "Received work slice [" << intervalStack[stackTop].first << ", " << intervalStack[stackTop].second
         << ") at level " << stackTop << endl;
    #endif

    algo->dataChanged();
    newSolutionFound = false;
    workSplitPossible = true;
}


void Computation::setSolution(opt_t optimum, vector<char> configuration, bool isAbsolute) {
    this->optimum = optimum;

    if(configuration.size() != (size_t)instanceSize) {
        throw "Attempted to set a configuration of wrong size as the current solution!";
    }

    memcpy(optimalConfig, configuration.data(), instanceSize);
    absoluteSolution = isAbsolute;
}


void Computation::reinitialize(int instanceSize, opt_t initialOptimum, char* initialConfiguration, int maxDepthLevel) {
    this->instanceSize = instanceSize;
    optimum = initialOptimum;
    stackTop = 0;
    maxStackSize = maxDepthLevel + 1;
    absoluteSolution = false;
    newSolutionFound = false;
    workSplitPossible = true;

    deallocateAll();

    configStack = new char[instanceSize * maxStackSize];
    intervalStack = new pair<int, int>[instanceSize * maxStackSize];
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
    delete[] intervalStack;
    delete[] optimalConfig;
}
