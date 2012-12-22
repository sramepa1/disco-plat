#include <iostream>
#include <fstream>
#include "Data.hh"
                                                                                
using namespace std;
                                                                                
class CRequestServiceA {
public:
   CRequestServiceA();
   ~CRequestServiceA();
   bool RequestServiceARoutineA();
   bool RequestServiceARoutineB();
                                                                                
   CosNaming::Name m_corbaCosName;
                                                                                
   // CORBA ORB
   CORBA::ORB_var             m_orb;
                                                                                
   CORBA::Object_var          m_obj;      // ORB Object
   CORBA::Object_var          m_obj1;     // Resolved id to object reference
                                                                                
   // Resolved and narrowed CORBA object for proxy calls
   Data::ServiceA_var         m_Data;
};
                                                                                
class DS_ServerConnectionException{
public:
   DS_ServerConnectionException() { cerr << "CORBA COMM_FAILURE" << endl; };
};
                                                                                
class DS_SystemException{
public:
   DS_SystemException() { cerr << "CORBA Exception" << endl; };
};
                                                                                
class DS_FatalException{
public:
   DS_FatalException() { cerr << "CORBA Fatal Exception" << endl; };
};
                                                                                
class DS_Exception{
public:
   DS_Exception() { cerr << "Exception" << endl; };
};

