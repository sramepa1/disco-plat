#include "Knapsack.h"

#include <cstring>

using namespace std;

void checkStream(istream& stream) {
    if(stream.fail()) {
        throw "Unable to correctly read from stream while constructing a Knapsack instance!";
    }
}

Knapsack::Knapsack(istream& textDataStream, Computation* comp) : AlgoInstance(comp) {

    textDataStream >> instanceSize;
    textDataStream >> capacity;
    checkStream(textDataStream);

    cost.resize(instanceSize);
    maxPossibleCost.resize(instanceSize);
    weight.resize(instanceSize);

    configBuffer = new char[instanceSize];
    memset(configBuffer, 0, instanceSize);
    comp->reinitialize(instanceSize, 0, configBuffer);

    for(unsigned int i = 0; i < instanceSize; i++) {
        textDataStream >> weight[i];
        textDataStream >> cost[i];
        checkStream(textDataStream);
    }

    unsigned int sumc = 0;
    for(int i = instanceSize - 1; i >= 0; i--) {
        sumc += cost[i];
        maxPossibleCost[i] = sumc;
    }
}

AlgoInstance* Knapsack::knapsackConstructor(istream& textDataStream, Computation* comp) {
    return new Knapsack(textDataStream, comp);
}

bool Knapsack::evaluate() {

	pair<int, int> interval;
	char* configuration;
	comp->peekState(configuration, interval);


	int sumw = 0;
	int sumc = 0;

    for(unsigned int i = 0; i < instanceSize; i++) {
        if(configuration[i]) {
            sumw += weight[i];
            sumc += cost[i];
        }
    }

	if(sumw > capacity) {
		// Overloaded knapsack. Cut branch.
		return false;
	}

	if((opt_t)sumc > comp->getOptimum()) {
		comp->newSolution(sumc, configuration);
	}

	for(int i = interval.first; i < interval.second; i++) {

		// all previous ones occupied and from this point,
		// it can't get any better. Cut branch.
		if(sumc + maxPossibleCost[i] < comp->getOptimum()) {
			return false;
		}

		// there is at least one unoccupied item slot.
		// branching is possible
		if(!configuration[i]) {
			return true;
		}
	}

	// all interval-specified options exhausted. Cut branch.
	return false;
}

void Knapsack::expand() {
	pair<int, int> interval;
	char* configuration;
	comp->peekState(configuration, interval);
	memcpy(configBuffer, configuration, instanceSize);

	for(int i = interval.first; i < interval.second; i++) {

		if(!configBuffer[i]) {

			// nudge left bound to start at a different item next time when backtracking to current state
			interval.first += 1;
			comp->setInterval(interval);

			// create a new state to push, with a matching "one to the right -> end" interval
			interval.first = i + 1;
			interval.second = instanceSize;
			configBuffer[i] = 1;
			comp->pushState(configBuffer, interval);
			return;
		}
	}

	throw "Expansion attempted in a non-expandable state! This must be an algorithm bug or memory corruption!";
}

void Knapsack::printConfig(char* configuration, ostream& os) {
    os << "Items: ";
    int sumw = 0;
    int sumc = 0;
    for(unsigned int i = 0; i < instanceSize; i++) {
        if(configuration[i]) {
            os << "1 ";
            sumw += weight[i];
            sumc += cost[i];
            continue;
        }
        os << "0 ";
    }
    os << " Cost: " << sumc << "  Weight: " << sumw;
    if(sumw > capacity) {
        os << " (OVERWEIGHT!)";
    }
    os << endl;
}
