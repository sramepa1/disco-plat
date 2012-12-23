
#include <iostream>
#include <string>

#include "Interface.h"

using namespace std;

int main(int argc, char** argv) {
    
    // initialization
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "mico-local-orb");

    CORBA::Object_var obj = orb->bind("IDL:StringSender:1.0", argv[1]);
    if (CORBA::is_nil(obj)) {
        cout << "CLIENT - cannot bind to " << argv[1] << endl;
        return 1;
    }
    
    StringSender_var strSender = StringSender::_narrow(obj);
    string strToSend;
    
    do {
        cout << "CLIENT - write something: ";
        getline(cin, strToSend);
        strSender->sendString(strToSend.c_str());
    } while(strToSend != "exit");
    
    return 0;
}