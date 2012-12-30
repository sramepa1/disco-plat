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

    virtual ~Knapsack() {}

    virtual void expand();

    virtual void printConfig(char* configuration, std::ostream& os);

protected:
    std::vector<int> cost;
    std::vector<int> weight;
};

#endif // KNAPSACK_H
