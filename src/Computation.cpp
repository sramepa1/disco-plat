#include "Computation.h"
#include "Synchronization.h"
#include "Algo.h"

#include <unistd.h>

using namespace std;

Computation::Computation() : algo(NULL) {
}

Computation::~Computation() {

}


/*
 *  WORK IN PROGRESS, THIS WILL LIKELY CHANGE A LOT!
 */


void Computation::setAlgorithm(AlgoInstance *algo) {
    this->algo = algo;
}


void Computation::setSync(Synchronization* sync) { /*TODO: implement*/}



void Computation::start() {
    sleep(60); // TODO: implement
}


void Computation::requestWork() { /*TODO: implement*/}


void Computation::setWork(const char* configStack, const std::pair<int,int> * intervalStack, int depth) { /*TODO: implement*/}


void Computation::setSolution(opt_t localOptimum, void* data) { /*TODO: implement*/}

void Computation::signalTrivialSolution(opt_t localOptimum, void* data) { /*TODO: implement*/}


void Computation::reinitialize(int instanceSize) { /*TODO: implement*/}

void Computation::newSolution(opt_t optimum, char* configuration) { /*TODO: implement*/}

/**
 * Changes the "next" node to examine at the top of the stack
 */
void Computation::setInterval(std::pair<int,int> interval) { /*TODO: implement*/}

void Computation::peekState(char* & configuration, std::pair<int,int> & interval) { /*TODO: implement*/}
void Computation::popState(char* & configuration, std::pair<int,int> & interval) { /*TODO: implement*/}
void Computation::pushState(char* configuration, std::pair<int,int> interval) { /*TODO: implement*/}
