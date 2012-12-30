
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
    {NULL, 0, NULL, 0}
};

void startNetwork(int port, const char* networkInterface, const char* algorithm, const char* address) {
    networkModule = new Network(port, networkInterface, algorithm);
    repo = new Repository();
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
    const char* name = "ham0";
    const char* file = NULL;

    int character;
    int option_index = 0;

    while ((character = getopt_long(argc, argv, "p:a:n:f:", long_options, &option_index)) != -1) {
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

            default:
                cerr << USAGE_INFO << endl;
                return ERR_INVALID_ARGUMENTS;
        }
    }

    cout << "Initializing network module with port " << port << ", name '" << name << "' and address '"
         << (address == NULL ? "NULL" : address) << "'" << endl;

    AlgoFactory::registerAlgorithm("Knapsack", &Knapsack::knapsackConstructor);

    try {

        string algoName;
        ifstream inFile;
        unsigned int initialID;

        if(file) {
            inFile.open(file);
            if(inFile.fail()) {
                throw (string("Unable to open file ") + file).c_str();
            }
            getline(inFile, algoName);
            if(!inFile.good()) {
                throw (string("Error reading file ") + file).c_str();
            }

            startNetwork(port, name, algoName.c_str(), address);

            initialID = repo->getFreeID();
            repo->newData(initialID, algoName,
                          string(istreambuf_iterator<char>(inFile), istreambuf_iterator<char>()),
                            // range-constructed string containing almost the entire input file
                            // (from current position to end-of-stream (default constructor)
                          true);

        } else {

            if(!address) {
                throw "Cannot start with neither an address to join nor an instance file!";
            }

            startNetwork(port, name, "", address);
            initialID = repo->getAnyValidID();
        }

        repo->start(initialID);
        // TODO: attempt to start a different computation if available

        // cleanup
        delete networkModule;

    } catch(const char* str) {
        cerr << "Error: " << str << endl;
    }

    return 0;
}
