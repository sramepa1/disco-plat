#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include "globals.h"
#include "NeighbourIface.h"
#include "Synchronization.h"

class Synchronization
{

    LeftNeighbourIface* leftNb;
    RightNeighbourIface* rightNb;

    unsigned int computationID;

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
    void newResult(const char* data, int dataLenght);

    /**
     * Blocks until new work arrives or computation terminates.
     */
    void workDone() {}




    void informRequest(disco_plat::nodeID requesteeID) {}
    void informResult(const char* data, int dataLenght) {}
    void informTerminate() {}

};

#endif // SYNCHRONIZATION_H
