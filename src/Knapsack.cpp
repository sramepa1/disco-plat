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

    unsigned int sumc = 0;
    unsigned int itemCost;
    for(unsigned int i = 0; i < instanceSize; i++) {
        textDataStream >> weight[i];
        textDataStream >> itemCost;
        cost[i] = itemCost;
        sumc += itemCost;
        maxPossibleCost[i] = sumc;
        checkStream(textDataStream);
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
		return false;
	}

	if((opt_t)sumc > comp->getOptimum()) {
		comp->newSolution(sumc, configuration);
	}

/*
	for(long i = depth; i < instsize; i++) {
		if(cost + maxpossiblecost[i] < bestcost)
			return;	// BB2
		knapdfs(bitvec | (1 << i), depth+i+1);
	}
*/
	return false; // TODO: remove
}

void Knapsack::expand() {
	// TODO:
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
