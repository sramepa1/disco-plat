#ifndef COMPUTATION_H
#define COMPUTATION_H

#include <utility>
#include <vector>

extern "C" {
#include <stdint.h>
}

class Synchronization;
class AlgoInstance;

class AbsoluteSolutionException {};


struct WorkUnit {
    int instanceSize;
    int depth;
    std::vector<char> configStackVector;
    std::vector<int> intervalStackVector;
};


typedef int opt_t;  // Integer-based optima are most common in NP-Hard problems. A little simplification won't hurt.

class Computation
{
public:

    Computation();

    virtual ~Computation();


    /**
     * Sets current algorithm. Doesn't take ownership.
     */
    void setAlgorithm(AlgoInstance* algo);

    /**
     * Sets current sync. Takes ownership.
     */
    void setSync(Synchronization* sync);

    /**
     * Gets this computation's synchronizer
     */
    Synchronization* getSync() { return sync; }


    // -------------- Outside world interface -------------------

    /**
     * Wrapper method for the searching algorithm. Executes BB-DFS. Requests work first if localStart is false.
     */
    void start(bool localStart);

    /**
     * Indicates if a new solution is available and should be extracted by getSolution()
     */
    bool hasNewSolution() { return newSolutionFound; }

    /**
     * Indicates if the current best known solution is an absolute one and no further work needs to be done.
     */
    bool isSolutionAbsolute() { return absoluteSolution; }

    /**
     * Extracts the current best known solution. Lowers the "hasNewSolution" flag if it was set.
     */
    std::pair<opt_t, std::vector<char> > getSolution();

    /**
     * Attempts to split avaliable work in two. May return false if no work is available.
     */
    bool splitWork(WorkUnit& workToSend, WorkUnit& workToAnnounceKept);

    /**
     * Sets a new stack to work with.
     *
     * SHOULD ONLY BE CALLED WHEN MAIN THREAD IS BLOCKED AFTER FINISHING PREVIOUS WORK.
     */
    void setWork(WorkUnit& work);

    /**
     * Sets a new best solution found by another process.
     * If marked as absolute, immediate termination will occur.
     */
    void setSolution(opt_t optimum, std::vector<char> configuration, bool isAbsolute);

    /**
     * Returns true if first opt_t operand is better than the second one;
     */
    bool isBetter(opt_t thisOptimum, opt_t thanThisOptimum);


    // -------------- Algorithm interface -------------------

    /**
     * Flushes stacks and resets environment. Called when pairing an AlgoInstance with this Computation.
     */
    void reinitialize(int instanceSize, opt_t initialOptimum, char* initialConfiguration, int maxDepthLevel);

    opt_t getOptimum() { return optimum; }

    int getDepthLevel() { return stackTop; }

    char* accessConfigAtDepth(int depth) { return configStack + depth * instanceSize; }
    std::pair<int, int>& accessIntervalAtDepth(int depth) { return intervalStack[depth]; }

    void newSolution(opt_t optimum, char* configuration, bool isAbsolute);

    /**
     * Changes the "next" node to examine at the top of the stack
     */
    void setInterval(std::pair<int,int> interval);

    void peekState(char* & configuration, std::pair<int,int> & interval);
    void popState(char* & configuration, std::pair<int,int> & interval);
    void pushState(char* configuration, std::pair<int,int> interval);



private:
    AlgoInstance* algo;
    Synchronization* sync;

    int instanceSize; // determines config length and maximum stack depths

    char* configStack;
    std::pair<int,int>* intervalStack;

    int stackTop;   // shared for both stacks
    int maxStackSize;

    // current best known solution
    opt_t optimum;
    char* optimalConfig;

    bool newSolutionFound;  // flag to broadcast my solution upon next synchronization
    bool absoluteSolution;  // flag to use when an absolute solution is found (be it here or elsewhere)
    bool workSplitPossible; // flag to avoid evaluating work-split possibility when already past the point of no return

    int loopCounter;
    int loopsToSync;    // Count of DFS iterations betwen synchronizations.
    uint64_t timestamp;

    void DFS();
    void synchronize();

    void deallocateAll();

    uint64_t areWeThereYet();

};

#endif // COMPUTATION_H
