CC            = g++
CPPFLAGS      = -g -c
LDFLAGS       = -g
LIBS          = -lmico2.3.13 -lpthread -ldl -lssl -lcrypto -lz
OBJECTS       = Interface.o Node.o
IDL           = idl
                                                                                
all Node: $(OBJECTS)
	$(CC) $(LDFLAGS) -o Node $(OBJECTS) $(LIBS)
                                                                                
Node.o: Node.cpp Interface.h
	$(CC) $(CPPFLAGS) Node.cpp
                                                                          
Interface.o: Interface.cc Interface.h
	$(CC) $(CPPFLAGS) -I. Interface.cc
                                                                                
Interface.cc: Interface.idl
	$(IDL) Interface.idl
                                                                                
clean clean_all:
	rm -fr *.o
	rm -fr *.h
	rm -fr *.cc
	rm -fr Node
