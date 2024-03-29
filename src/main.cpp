
#include "globals.h"
#include "Network.h"
#include "Synchronization.h"
#include "Repository.h"
#include "Algo.h"
#include "Computation.h"
#include "Knapsack.h"

#include <iostream>

extern "C" {
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
}


using namespace std;

// global vars definition
Network* networkModule;
Synchronization* currentSyncModule;

static struct option long_options[] = {
    {"port", 1, 0, 'p'},
    {"address", 1, 0, 'a'},
    {"name", 1, 0, 'n'},
    {"file", 1, 0, 'f'},
    {"output", 1, 0, 'o'},
    {NULL, 0, NULL, 0}
};

void initEnvironment(int port, const char* networkInterface, const char* algorithm,
                     const char* address, const char* outFile) {

    #ifdef VERBOSE
    cout << "Initializing network module with port " << port << ", interface '" << networkInterface
         << "' and address '" << (address == NULL ? "NULL" : address) << "'" << endl;
    #endif

    networkModule = new Network(port, networkInterface, algorithm);
    repo = new Repository(outFile);
    networkModule->start(address);
    repo->init();
}

int main(int argc, char** argv) {

    /*
    if(argc < 2) {
        cerr << USAGE_INFO << endl;
        return ERR_INVALID_ARGUMENTS;
    }
    */

    int port = DEFAULT_PORT;
    const char* address = NULL;
    const char* name = "eth0";
    const char* file = NULL;
    const char* output = "";

    int character;
    int option_index = 0;

    while ((character = getopt_long(argc, argv, "p:a:n:f:o:", long_options, &option_index)) != -1) {
        switch (character) {
            case 'p' :
                port = atoi(optarg);
                break;

            case 'a' :
                address = strdup(optarg);
                break;

            case 'n' :
                name = strdup(optarg);
                break;

            case 'f' :
                file = strdup(optarg);
                break;

            case 'o' :
                output = strdup(optarg);
                break;

            default:
                cerr << USAGE_INFO << endl;
                return ERR_INVALID_ARGUMENTS;
        }
    }

    AlgoFactory::registerAlgorithm("Knapsack", &Knapsack::knapsackConstructor);

    try {

        string algoName;
        ifstream inFile;
        unsigned int computationID;

        if(file) {
            inFile.open(file);
            if(inFile.fail()) {
                throw (string("Unable to open file ") + file).c_str();
            }
            getline(inFile, algoName);
            if(!inFile.good()) {
                throw (string("Error reading file ") + file).c_str();
            }

            initEnvironment(port, name, algoName.c_str(), address, output);

            computationID = repo->getFreeID();
            repo->newData(computationID, algoName,
                          string(istreambuf_iterator<char>(inFile), istreambuf_iterator<char>()),
                            // range-constructed string containing almost the entire input file
                            // (from current position to end-of-stream (default constructor)
                          true);

        } else {

            if(!address) {
                throw "Cannot start with neither an address to join nor an instance file!";
            }

            initEnvironment(port, name, "", address, output);
            computationID = repo->getAnyValidID();
        }

        bool localStart = file != NULL;

        do {
            repo->startComputation(computationID, localStart);

            localStart = false;
            repo->destroyComputation(computationID);
            computationID = repo->getAnyValidID();

        } while(computationID != INVALID_COMPUTATION_ID);

        // cleanup
        delete networkModule;

    } catch(const char* str) {
        cerr << "Error: " << str << endl;
    }

    return 0;
}
