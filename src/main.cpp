
#include "Network.h"

#include <iostream>

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#define DEFAULT_PORT 1234
#define ERR_INVALID_ARGUMENTS 1
#define USAGE_INFO "usage: disco-plat port [address]"


using namespace std;

static struct option long_options[] = {
    {"port", 1, 0, 'p'},
    {"address", 1, 0, 'a'},
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

    int character;
    int option_index = 0;

    while ((character = getopt_long(argc, argv, "p:a:", long_options, &option_index)) != -1) {
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

            default:
                cerr << USAGE_INFO << endl;
                return ERR_INVALID_ARGUMENTS;
        }
    }


    Network network(port, address);


    
    return 0;
}
