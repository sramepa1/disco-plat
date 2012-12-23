CC            = g++
CPPFLAGS      = -g -c
LDFLAGS       = -g
LIBS          = -lmico2.3.13 -lpthread -ldl
OBJECTS       = Interface.o Client.o
IDL           = idl
                                                                                
all Client: $(OBJECTS)
	$(CC) $(LDFLAGS) -o Client $(OBJECTS) $(LIBS)
                                                                                
Client.o: Client.cpp Interface.h
	$(CC) $(CPPFLAGS) Client.cpp
                                                                          
Interface.o: Interface.cc Interface.h
	$(CC) $(CPPFLAGS) -I. Interface.cc
                                                                                
Interface.cc: ../Interface.idl
	$(IDL) ../Interface.idl
                                                                                
clean clean_all:
	rm -fr *.o
	rm -fr *.h
	rm -fr *.cc
	rm -fr Client
