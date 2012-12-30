#ifndef COMPUTATION_H
#define COMPUTATION_H

#include <utility>

class Synchronization;
class AlgoInstance;


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

    void setSync(Synchronization* sync);


    // -------------- Outside world interface -------------------

    /**
     * Wrapper method for the searching algorithm. Executes BB-DFS.
     */
    void start();

    /**
     * Requests a stack slice to be sent when possible.
     * Multiple calls increment a counter and are then replied to in bulk.
     */
    void requestWork();

    /**
     * Sets a new stack to work with.
     *
     * SHOULD ONLY BE CALLED WHEN MAIN THREAD IS BLOCKED AFTER FINISHING PREVIOUS WORK.
     */
    void setWork(const char* configStack, const std::pair<int,int> * intervalStack, int depth);

    /**
     * Sets a new best solution found by another process.
     */
    void setSolution(opt_t localOptimum, void* data);

    /**
     * Request a controlled termination of this process on the first possible occasion
     * because another process found a trivial solution.
     */
    void signalTrivialSolution(opt_t localOptimum, void* data);


    // -------------- Algorithm interface -------------------

    void reinitialize(int instanceSize);

    opt_t getOptimum() { return optimum; }

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


    bool trivialSolution; // flag to use when a trivial solution is found elsewhere

    int workRequestCounter;
    int loopsToSync;    // Count of DFS iterations betwen synchronizations.

};

#endif // COMPUTATION_H
