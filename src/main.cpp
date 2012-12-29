
#include "globals.h"
#include "Network.h"

#include <iostream>

extern "C" {
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
}

#define DEFAULT_PORT 1234
#define ERR_INVALID_ARGUMENTS 1
#define USAGE_INFO "usage: disco-plat port [address]"


using namespace std;

// global vars definition
Network* networkModule;

static struct option long_options[] = {
    {"port", 1, 0, 'p'},
    {"address", 1, 0, 'a'},
    {"name", 1, 0, 'n'},
    {"file", 1, 0, 'f'},
    {NULL, 0, NULL, 0}
};

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
    const char* algoritm = NULL;
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

    // create connection
    networkModule = new Network(port, name, algoritm);
    networkModule->start(address);

    // some useful work... :)
    sleep(60);

    // network cleanup
    delete networkModule;

    return 0;
}
