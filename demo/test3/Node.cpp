
#define PORT 15672

#include <iostream>
#include <string>
#include <sstream>

#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netdb.h>

#include "Interface.h"

using namespace std;

// globals
int myPort;
string otherAddr;

bool canSend;

class NetworkImpl : virtual public POA_Network {
    
    void joinNetwork(const char* addr) {
        otherAddr = addr;
    }
    
    void sendInteger(CORBA::Long number) {
        cout << "Node (port " << myPort << ") - recieved number: " << number << endl;
        canSend = true;
    }
};


int main(int argc, char** argv) {

    bool haveOther;
    utsname myUname;
    uname(&myUname);
    
    if(argc == 1) {
        // first node
        haveOther = false;
        canSend = false;
        myPort = PORT;
    } else {
        // second node
        haveOther = true;
        canSend = true;
        myPort = PORT + 1;
    }
    
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        return 1;
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        
        if (ifa->ifa_addr == NULL)
            continue;  

        s = getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if((strcmp(ifa->ifa_name, "ham0") == 0) && (ifa->ifa_addr->sa_family == AF_INET)) {
            
            if(s != 0) {
                return 1;
            }
            break;
        }
    }

    freeifaddrs(ifaddr);
    
    stringstream thisAddr;
    thisAddr << "inet:" << host << ":" << myPort;
    
    cout << "Node (port " << myPort << ") - my address: " << thisAddr.str() << endl;
    
    int argcORB = 3;
    char** argvORB = new char*[argcORB];
    argvORB[0] = argv[0];
    
    argvORB[1] = new char[strlen("-ORBIIOPAddr") + 1];
    memcpy(argvORB[1], "-ORBIIOPAddr", strlen("-ORBIIOPAddr") + 1);
    
    argvORB[2] = new char[thisAddr.str().size() + 1];
    memcpy(argvORB[2], thisAddr.str().c_str(), thisAddr.str().size() + 1);
    
    
    // initialization
    CORBA::ORB_var orb = CORBA::ORB_init(argcORB, argvORB, "mico-local-orb");
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);
    PortableServer::POAManager_var mgr = poa->the_POAManager();
    
    NetworkImpl* myObject = new NetworkImpl;
    poa->activate_object(myObject);
    myObject->_this();
    
    
    Network_var otherObject;
    
    if(argc != 1) {
        // create connection to first node
        CORBA::Object_var otherObj = orb->bind("IDL:Network:1.0", argv[1]);
        if (CORBA::is_nil(otherObj)) {
            cout << "Node (port " << myPort << ") - cannot bind to " << argv[1] << endl;
            return 1;
        }
        
        otherObject = Network::_narrow(otherObj);
        
        stringstream thisUnameAddr;
        thisUnameAddr << "inet:" << myUname.nodename << ":" << myPort;
        otherObject->joinNetwork(thisUnameAddr.str().c_str());
    }
    
    
    // main loop
    mgr->activate();
    cout << "Node (port " << myPort << ") - starting main loop, press CTRL+C for exit!" << endl;
    while(true) {
        
        for(int i = 0; i < 1000; ++i) {
            if(orb->work_pending()) {
                orb->perform_work();
                
                if(!haveOther && !otherAddr.empty()) {
                    CORBA::Object_var otherObj = orb->bind("IDL:Network:1.0", otherAddr.c_str());
                    if (CORBA::is_nil(otherObj)) {
                        cout << "Node (port " << myPort << ") - cannot bind to " << otherAddr << endl;
                        return 1;
                    }
        
                    otherObject = Network::_narrow(otherObj);
                    haveOther = true;
                }
            }
        }
        
        int randomNumber = rand();
        if(canSend && haveOther && randomNumber % 50 == 0) {
            cout << "Node (port " << myPort << ") - sending number: " << randomNumber << endl;
            canSend = false;
            otherObject->sendInteger((CORBA::Long)randomNumber);
        }
    }

    return 0;
}
