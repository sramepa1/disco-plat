#include "CRequestServiceA.h"
                                                                                
int main(int argc, char** argv)
{
   CRequestServiceA requestServiceA;  // Constructor establishes the link with the CORBA server.
                                                                                
   if(requestServiceA.RequestServiceARoutineA()) cout << "ServiceA RoutineA: True" << endl;
   if(requestServiceA.RequestServiceARoutineB()) cout << "ServiceA RoutineB: True" << endl;
                                                                                
   return 0;
}

