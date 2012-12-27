#include "Network.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cstring>

#include <sys/utsname.h>

#include "../build/Interface.h"

using namespace std;
using namespace disco_plat;


Network::Network(int port, const char* remoteAddr) {

    utsname myUname;
    uname(&myUname);

    stringstream thisAddr;
    thisAddr << "inet:" << myUname.nodename << ":" << port;

    cout << "Node (address: " << thisAddr.str() << ") - initializing" << endl;

    int argcORB = 3;
    char** argvORB = new char*[argcORB];

    argvORB[0] = new char[strlen("disco-plat") + 1];
    memcpy(argvORB[1], "disco-plat", strlen("disco-plat") + 1);

    argvORB[1] = new char[strlen("-ORBIIOPAddr") + 1];
    memcpy(argvORB[1], "-ORBIIOPAddr", strlen("-ORBIIOPAddr") + 1);

    argvORB[2] = new char[thisAddr.str().size() + 1];
    memcpy(argvORB[2], thisAddr.str().c_str(), thisAddr.str().size() + 1);

}


void* Network::recvThreadMain(void* ptr) {
    Network* instance = (Network*)ptr;
    return NULL;
}


void* Network::sendThreadMain(void* ptr) {
    Network* instance = (Network*)ptr;
    return NULL;
}
