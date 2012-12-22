CC            = g++
CPPFLAGS      = -g -c
LDFLAGS       = -g
LIBS          = -lomniORB4 -lomnithread -lomniDynamic4
OBJECTS       = DataSK.o Client.o CRequestServiceA.o
                                                                                
all Client: $(OBJECTS)
	$(CC) $(LDFLAGS) -o Client $(OBJECTS) $(LIBS)
                                                                                
Client.o: Client.cpp CRequestServiceA.h
	$(CC) $(CPPFLAGS) Client.cpp
                                                                                
CRequestServiceA.o: CRequestServiceA.cpp CRequestServiceA.h Data.hh
	$(CC) $(CPPFLAGS) CRequestServiceA.cpp
                                                                                
DataSK.o: DataSK.cc Data.hh
	$(CC) $(CPPFLAGS) DataSK.cc
                                                                                
DataSK.cc: ../Data.idl
	omniidl -bcxx ../Data.idl
                                                                                
clean clean_all:
	rm -fr *.o
	rm -fr core
	rm -fr *.hh
	rm -fr *SK.cc
	rm -fr Client
