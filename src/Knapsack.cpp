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
    costCache.resize(instanceSize);
    weightCache.resize(instanceSize);

    configBuffer = new char[instanceSize];
    memset(configBuffer, 0, instanceSize);
    comp->reinitialize(instanceSize, 0, configBuffer);
    costCache[0] = 0;

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

	int depth = comp->getDepthLevel();

	int thisCost = costCache[depth];
	int thisWeight = weightCache[depth];
	int optimum = (int)comp->getOptimum();

	for(int i = interval.first; i < interval.second; i++) {

		// all previous ones occupied and from this point,
		// it can't get any better. Cut branch.
		if(thisCost + maxPossibleCost[i] < optimum) {
			return false;
		}

		// there is at least one unoccupied item slot.
		// branching is possible
		if(!configuration[i] && thisWeight + weight[i] <= capacity) {
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

	int depth = comp->getDepthLevel();
	int thisWeight = weightCache[depth];

	for(int i = interval.first; i < interval.second; i++) {

		if(!configBuffer[i] && thisWeight + weight[i] <= capacity) {

			// nudge left bound to start at a different item next time when backtracking to current state
			interval.first += 1;
			comp->setInterval(interval);

			// create a new state to push, with a matching "one to the right -> end" interval
			interval.first = i + 1;
			interval.second = instanceSize;
			configBuffer[i] = 1;
			comp->pushState(configBuffer, interval);

			// update caches and check for new solutions
			int newCost = costCache[depth] + cost[i];
			costCache[depth + 1] = newCost;
			weightCache[depth + 1] = weightCache[depth] + weight[i];

			if((opt_t)newCost >= comp->getOptimum()) {
				comp->newSolution(newCost, configBuffer, depth == (int)instanceSize - 1);
			}

			return;
		}
	}

	throw "Expansion attempted in a non-expandable state! This must be an algorithm bug or memory corruption!";
}

void Knapsack::dataChanged() {
	for(int i = 0; i <= comp->getDepthLevel(); i++) {
		char* config = comp->accessConfigAtDepth(i);
		int sumc;
		int sumw;
		for(unsigned int j = 0; j < instanceSize; j++) {
			if(config[j]) {
				sumc += cost[j];
				sumw += weight[j];
			}
		}
		costCache[i] = sumc;
		weightCache[i] = sumw;
	}
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
