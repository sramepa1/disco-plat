CC            = g++
CPPFLAGS      = -g -c
LDFLAGS       = -g
LIBS          = -lmico2.3.13 -lpthread -ldl
OBJECTS       = Interface.o Server.o
IDL           = idl
                                                                                
all Server: $(OBJECTS)
	$(CC) $(LDFLAGS) -o Server $(OBJECTS) $(LIBS)
                                                                                
Server.o: Server.cpp Interface.h
	$(CC) $(CPPFLAGS) Server.cpp
                                                                          
Interface.o: Interface.cc Interface.h
	$(CC) $(CPPFLAGS) -I. Interface.cc
                                                                                
Interface.cc: ../Interface.idl
	$(IDL) ../Interface.idl
                                                                                
clean clean_all:
	rm -fr *.o
	rm -fr *.h
	rm -fr *.cc
	rm -fr Server
