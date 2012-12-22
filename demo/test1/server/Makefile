CC            = g++
CPPFLAGS      = -g -c
LDFLAGS       = -g
LIBS          = -lomniORB4 -lomnithread -lomniDynamic4
OBJECTS       = DataSK.o CServiceA.o Server.o
                                                                                
all Server: $(OBJECTS)
	$(CC) $(LDFLAGS) -o Server $(OBJECTS) $(LIBS)
                                                                                
DataSK.o: DataSK.cc Data.hh
	$(CC) $(CPPFLAGS) DataSK.cc
                                                                                
Server.o: Server.cpp Data.hh
	$(CC) $(CPPFLAGS) Server.cpp
                                                                                
CServiceA.o: CServiceA.cpp CServiceA.h Data.hh
	$(CC) $(CPPFLAGS) CServiceA.cpp
                                                                                
DataSK.cc: ../Data.idl
	omniidl -bcxx ../Data.idl
                                                                                
clean clean_all:
	rm -fr *.o
	rm -fr core
	rm -fr *.hh
	rm -fr *SK.cc
	rm -fr Server
        
