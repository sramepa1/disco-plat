#ifndef KNAPSACK_H
#define KNAPSACK_H

#include "Algo.h"
#include "Computation.h"

#include <vector>

class Knapsack : public AlgoInstance
{
public:
    Knapsack(std::istream& textDataStream, Computation* comp);
    static AlgoInstance* knapsackConstructor(std::istream& textDataStream, Computation* comp);

    virtual ~Knapsack() { delete configBuffer; }

    virtual bool evaluate();
    virtual void expand();

    virtual void dataChanged();

    virtual void printConfig(char* configuration, std::ostream& os);
    virtual bool isBetter(opt_t thisOptimum, opt_t thanThisOptimum) { return thisOptimum > thanThisOptimum; }

protected:
    unsigned int instanceSize;

    int capacity;
    std::vector<int> cost;
    std::vector<int> weight;
    std::vector<int> maxPossibleCost;

    char* configBuffer;

    std::vector<int> costCache;
    std::vector<int> weightCache;
};

#endif // KNAPSACK_H
