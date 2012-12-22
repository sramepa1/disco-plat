#include <cassert>
#include "Data.hh"
#include "CRequestServiceA.h"

using namespace Data;
                                                                                
CRequestServiceA::CRequestServiceA()
{
  try {
    //------------------------------------------------------------------------
    // Initialize ORB object.
    //------------------------------------------------------------------------
    int    argc=0;       // Dummy variables to support following call.
    char** argv=0;
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
                                                                                
    //------------------------------------------------------------------------
    // Bind ORB object to name service object.
    // (Reference to Name service root context.)
    //------------------------------------------------------------------------
    cout << "aaaaaaaaaaa" << endl;
    
    CORBA::Object_var obj = orb->resolve_initial_references("OmniNameService");
    assert (!CORBA::is_nil(obj.in()));
                                                                                                           
    //------------------------------------------------------------------------
    // Narrow this to the naming context (Narrowed reference to root context.)
    //------------------------------------------------------------------------
    cout << "bbbbbbbbbbb" << endl;
    
    CosNaming::NamingContext_var nc =
                        CosNaming::NamingContext::_narrow(obj.in());
    assert (!CORBA::is_nil(nc.in()));
                                                                                
    //------------------------------------------------------------------------
    // The "name text" put forth by CORBA server in name service.
    // This same name ("DataServiceName1") is used by the CORBA server when
    // binding to the name server (CosNaming::Name).
    //------------------------------------------------------------------------
    cout << "cccccccccccc" << endl;
    
    CosNaming::Name _corbaCosName;
    _corbaCosName.length(1);
    _corbaCosName[0].id=CORBA::string_dup("DataServiceName1");
                                                                                
    //------------------------------------------------------------------------
    // Resolve "name text" identifier to an object reference.
    //------------------------------------------------------------------------
    cout << "ddddddddddddd" << endl;
    
    CORBA::Object_var obj1 = nc->resolve(_corbaCosName);
    assert(!CORBA::is_nil(obj1.in()));
                                                                                
    m_Data = ServiceA::_narrow(obj1.in());
    if (CORBA::is_nil(m_Data.in()))
    {
       cerr << "IOR is not an SA object reference." << endl;
    }
  }
  catch(CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
         << "object." << endl;
    throw DS_ServerConnectionException();
    return;
  }
  catch(CORBA::SystemException& ) {
    cerr << "Caught a CORBA::SystemException." << endl;
    throw DS_SystemException();
    return;
  }
  catch(CORBA::Exception& ) {
    cerr << "Caught CORBA::Exception." << endl;
    throw DS_Exception();
    return;
  }  catch(omniORB::fatalException& fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
    throw DS_FatalException();
    return;
  }
  catch(...) {
    cerr << "Caught unknown exception." << endl;
    throw DS_Exception();
    return;
  }
  return;
}
                                                                                
CRequestServiceA::~CRequestServiceA()
{
   // ...
}
                                                                                
bool CRequestServiceA::RequestServiceARoutineA()
{
   CORBA::Long num1=4;
   CORBA::Long num2=5;
   CORBA::Long retNum;
                                                                                
   cout << "Values input to Service Routine A: "
        << num1 << " "
        << num2 << " "
        << retNum << endl;
                                                                                
   if( m_Data->CallServiceRoutineA( num1, num2, retNum)) // This is the CORBA call which is to be executed remotely
   {    // Ok
      cout << "Values returned by Service Routine A: "
           << num1 << " "
           << num2 << " "
           << retNum << endl;
                                                                                
      return true;
   }
   else // fail
   {
      return false;
   }
                                                                                
   return true;
}
                                                                                
bool CRequestServiceA::RequestServiceARoutineB()
{
   CORBA::Long num1=0;
   CORBA::Long num2=50;
                                                                                
   cout << "Values input to Service Routine B: "
        << num1 << " "
        << num2 << endl;
                                                                                
   if( m_Data->CallServiceRoutineB( num1, num2)) // This is the CORBA call which is to be executed remotely
   {    // Ok
      cout << "Values returned by Service Routine B: "
           << num1 << " "
           << num2 << endl;
                                                                                
      return true;
   }
   else // fail
   {
      return false;
   }
                                                                                
   return true;
}

