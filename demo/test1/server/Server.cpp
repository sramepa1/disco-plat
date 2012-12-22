#include <stdlib.h>
#include <iostream>
#include <string>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
                                                                                
#include "CServiceA.h"
                                                                                
#include "Data.hh"

using namespace std;
                                                                                
int main(int argc, char** argv)
{
  // --------------------------------------------------------------------------
  // Start CORBA server:
  // --------------------------------------------------------------------------
                                                                                
  try {
    //------------------------------------------------------------------------
    // 1) Initialize ORB
    // 2) Get reference to root POA
    // 3) Bind to name service
    // 4) Initialize servant object
    //------------------------------------------------------------------------
                                                                                
    //------------------------------------------------------------------------
    // Initialize CORBA ORB - "orb"
    //------------------------------------------------------------------------
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
                                                                                
    //------------------------------------------------------------------------
    // Servant must register with POA in order to be made available for client
    // Get reference to the RootPOA.
    //------------------------------------------------------------------------
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var _poa = PortableServer::POA::_narrow(obj.in());
                                                                                
    //------------------------------------------------------------------------
    // Operations defined in object interface invoked via an object reference.
    // Instance of CRequestSocketStream_i servant is initialized.
    //------------------------------------------------------------------------
    CServiceA_i* myRequestServiceA = new CServiceA_i();
                                                                                
    //------------------------------------------------------------------------
    // ObjectId_var class defined in poa.h
    // typedef String_var ObjectId_var; CORBA_ORB.h
    // ???????
                                                                                
    //------------------------------------------------------------------------
    // Servant object activated in RootPOA.
    // (Object id used for various POA operations.)
    //------------------------------------------------------------------------
    PortableServer::ObjectId_var myRequestServiceA_oid
                                = _poa->activate_object(myRequestServiceA);
                                                                                
    //------------------------------------------------------------------------
    // Obtain object reference from servant and register in naming service(??)
    //------------------------------------------------------------------------
    CORBA::Object_var SA_obj = myRequestServiceA->_this();
                                                                                
    //------------------------------------------------------------------------
    // Obtain a reference to the object, and print it out as string IOR.
    //------------------------------------------------------------------------
    CORBA::String_var sior(orb->object_to_string(SA_obj.in()));
    cerr << "'" << (char*)sior << "'" << endl;
                                                                                
    //========================================================================
    // Bind (rebind) object (orb) to name (SA_obj)
    //========================================================================
                                                                                
    //------------------------------------------------------------------------
    // Bind object to name service as defined by directive InitRef
    // and identifier "OmniNameService" in config file omniORB.cfg.
    //------------------------------------------------------------------------
    CORBA::Object_var obj1=orb->resolve_initial_references("OmniNameService");
    assert(!CORBA::is_nil(obj1.in()));
                                                                                
    //------------------------------------------------------------------------
    // narrow this to the naming context
    //------------------------------------------------------------------------
    CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(obj1.in());
    assert(!CORBA::is_nil(nc.in()));
                                                                                
    //------------------------------------------------------------------------
    // Bind to CORBA name service. Same name to be requested by client.
    //------------------------------------------------------------------------
    CosNaming::Name name;
    name.length(1);
    name[0].id=CORBA::string_dup("DataServiceName1");
    nc->rebind (name,SA_obj.in());
                                                                                
    //========================================================================
                                                                                
    myRequestServiceA->_remove_ref();
                                                                                
    //------------------------------------------------------------------------
    // Activate the POA manager
    //------------------------------------------------------------------------
    PortableServer::POAManager_var pmgr = _poa->the_POAManager();
    pmgr->activate();
                                                                                
    //------------------------------------------------------------------------
    // Accept requests from clients
    //------------------------------------------------------------------------
    orb->run();
                                                                                
    //------------------------------------------------------------------------
    // If orb leaves event handling loop.
    // - currently configured never to time out (??)
    //------------------------------------------------------------------------
    orb->destroy();
                                                                                
    free(name[0].id); // str_dup does a malloc internally
  }
                                                                                
  catch(CORBA::SystemException&) {
    cerr << "Caught CORBA::SystemException." << endl;
  }
  catch(CORBA::Exception&) {
    cerr << "Caught CORBA::Exception." << endl;
  }
  catch(omniORB::fatalException& fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
  }
  catch(...) {
    cerr << "Caught unknown exception." << endl;
  }
                                                                                
  return 0;
}
