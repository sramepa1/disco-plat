#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

class Synchronization
{

    // DISABLED
    Synchronization(const Synchronization&) {}

public:
    Synchronization();
    ~Synchronization();

    /**
     * Checks if threre are any incoming messages and then calls their handlers.
     */
    void synchronize() {}

    /**
     * Globally broadcasts new best solution.
     * @param nodes array of nodes in dominant set
     * @param nodeCount count of nodes in domninant set
     */
    void newResult(const char* data, int dataLenght) {}

    /**
     * Blocks until new work arrives or computation terminates.
     */
    void workDone() {}

};

#endif // SYNCHRONIZATION_H
