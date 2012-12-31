#ifndef COMPUTATION_H
#define COMPUTATION_H

#include <utility>
#include <vector>

class Synchronization;
class AlgoInstance;

class TrivialSolutionException {};


typedef int opt_t;  // Integer-based optima are most common in NP-Hard problems. A little simplification won't hurt.

class Computation
{
public:

    Computation();

    virtual ~Computation();


    /*
     *  WORK IN PROGRESS, THIS WILL LIKELY CHANGE A LOT!
     */


    /**
     * Sets current algorithm. Doesn't take ownership.
     */
    void setAlgorithm(AlgoInstance* algo);

    /**
     * Sets current sync. Doesn't take ownership.
     */
    void setSync(Synchronization* sync);


    // -------------- Outside world interface -------------------

    /**
     * Wrapper method for the searching algorithm. Executes BB-DFS.
     */
    void start();

    /**
     * Indicates if a new solution is available and should be extracted by getSolution()
     */
    bool hasNewSolution() { return hasNewSolution(); } // TODO fix stack overflow!

    /**
     * Extracts the current best known solution. Lowers the "hasNewSolution" flag if it was set.
     */
    std::pair<opt_t, std::vector<char> > getSolution();

    /**
     * Attempts to split avaliable work for requestCount other nodes.
     * May return less slices than desired (or empty vector) if split is not possible.
     *
     * DATA TYPE OF VECTOR IS ONLY TEMPORARY FOR NOW!
     */
    std::vector<char*> splitWork(int requestCount);

    /**
     * Sets a new stack to work with.
     *
     * SHOULD ONLY BE CALLED WHEN MAIN THREAD IS BLOCKED AFTER FINISHING PREVIOUS WORK.
     */
    void setWork(const char* configStack, const std::pair<int,int> * intervalStack, int depth);

    /**
     * Sets a new best solution found by another process.
     * If marked as trivial, immediate termination will occur.
     */
    void setSolution(opt_t optimum, std::vector<char> configuration, bool isTrivial);

    /**
     * Returns true if first opt_t operand is better than the second one;
     */
    bool isBetter(opt_t thisOptimum, opt_t thanThisOptimum);


    // -------------- Algorithm interface -------------------

    /**
     * Flushes stacks and resets environment. Called when pairing an AlgoInstance with this Computation.
     */
    void reinitialize(int instanceSize, opt_t initialOptimum, char* initialConfiguration);

    opt_t getOptimum() { return optimum; }

    int getDepth() { return stackTop; }

    char* accessConfigAtDepth(int depth) { return configStack + depth * instanceSize; }
    std::pair<int, int>& accessIntervalAtDepth(int depth) { return intervalStack[depth]; }

    void newSolution(opt_t optimum, char* configuration);

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
    char* configStackCopy; // for work splitting

    std::pair<int,int>* intervalStack;
    std::pair<int,int>* intervalStackCopy;

    int stackTop;   // shared for both stacks

    // current best known solution
    opt_t optimum;
    char* optimalConfig;

    bool newSolutionFound;  // flag to broadcast my solution upon next synchronization
    bool trivialSolution;   // flag to use when a trivial solution is found elsewhere

    int loopsToSync;    // Count of DFS iterations betwen synchronizations.

    void DFS();
    void synchronize();

    void deallocateAll();

};

#endif // COMPUTATION_H
