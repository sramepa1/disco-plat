CPPFLAGS      = -Wall -pedantic -O0 -g -Ibuild -DVERBOSE
LDFLAGS       = -g
LIBS          = -lmico2.3.13 -lpthread -ldl -lssl -lcrypto -lz -lrt

OBJECTS = $(patsubst src/%.cpp,build/%.o,$(wildcard src/*.cpp))

all: disco-plat
	@echo "\n--------------------------------------------"

build:
	mkdir build

build/Interface.o: build/Interface.cc
	g++ $(CPPFLAGS) -c -o $@ $<

build/Interface.cc: build src/Interface.idl
	idl src/Interface.idl
	mv Interface.cc build/Interface.cc
	mv Interface.h build/Interface.h

build/%.o: src/%.cpp
	g++ $(CPPFLAGS) -c -o $@ $<

disco-plat: build/Interface.o $(OBJECTS)
	g++ $(LDFLAGS) -o $@ build/Interface.o $(OBJECTS) $(LIBS)

clean:
	rm -rf build
	rm -f disco-plat

.PHONY: clean


