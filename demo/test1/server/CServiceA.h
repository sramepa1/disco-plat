#include "Data.hh"
                                                                                
class CServiceA_i :  public POA_Data::ServiceA,
                     public PortableServer::RefCountServantBase
{
public:
   CServiceA_i();
   virtual ~CServiceA_i();
   virtual CORBA::Boolean CallServiceRoutineA( CORBA::Long num1,
                                               CORBA::Long& num2,
                                               CORBA::Long& retNum);
   virtual CORBA::Boolean CallServiceRoutineB( CORBA::Long& num1,
                                               CORBA::Long& num2);
};

