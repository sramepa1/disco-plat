
#include "globals.h"
#include "Network.h"

#include <iostream>

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

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
    char* address = NULL;
    char* name = (char*) "ham0";
    char* file = NULL;

    int character;
    int option_index = 0;

    while ((character = getopt_long(argc, argv, "p:a:n:f:", long_options, &option_index)) != -1) {
        switch (character) {
            case 'p' :
                port = atoi(optarg);
                break;

            case 'a' :
                {
                    int lenght = strlen(optarg);
                    address = new char[lenght + 1];
                    memcpy(address, optarg, lenght);
                    address[lenght] = 0;
                }

                break;

            case 'n' :
                {
                    int lenght = strlen(optarg);
                    name = new char[lenght + 1];
                    memcpy(name, optarg, lenght);
                    name[lenght] = 0;
                }

            case 'f' :
                {
                    int lenght = strlen(optarg);
                    file = new char[lenght + 1];
                    memcpy(file, optarg, lenght);
                    file[lenght] = 0;
                }

                break;

            default:
                cerr << USAGE_INFO << endl;
                return ERR_INVALID_ARGUMENTS;
        }
    }

    cout << "Initializing network module with port " << port << ", name '" << name << "' and address '"
         << (address == NULL ? "NULL" : address) << "'" << endl;

    // create connection
    networkModule = new Network(port, name, address);

    // some useful work... :)
    sleep(60);

    // network cleanup
    delete networkModule;
    
    return 0;
}
