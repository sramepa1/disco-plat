#!/bin/sh

MICORC=/dev/null
export MICORC

ADDR=inet:`uname -n`:15672

./server/Server -ORBIIOPAddr $ADDR &
sleep 1
./client/Client $ADDR

