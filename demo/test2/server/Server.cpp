
#include <iostream>

#include "Interface.h"

using namespace std;

bool running = true;

class StringSender_impl : virtual public POA_StringSender {
    void sendString(const char* s) {
        cout << "SERVER - recieved string: " << s << endl;
        
        if(!strcmp(s, "exit")) {
            running = false;
        }
    }
};

int main(int argc, char** argv) {
    
    // initialization
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "mico-local-orb");
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);
    PortableServer::POAManager_var mgr = poa->the_POAManager();
    
    StringSender_impl* strSender = new StringSender_impl;
    poa->activate_object(strSender);
    strSender->_this();
    
    mgr->activate();
    while(running) {
        orb->perform_work();
    }
    
    orb->shutdown(TRUE);
    
    return 0;
}