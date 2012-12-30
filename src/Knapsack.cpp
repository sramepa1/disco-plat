#include "Knapsack.h"

using namespace std;

Knapsack::Knapsack(istream& textDataStream, Computation *comp) : AlgoInstance(comp) {
    // TODO:
}

AlgoInstance *Knapsack::knapsackConstructor(istream &textDataStream, Computation *comp) {
    return new Knapsack(textDataStream, comp);
}

void Knapsack::expand() {
    // TODO:
}

void Knapsack::printConfig(char* configuration, ostream& os) {
    // TODO:
}
