#!/bin/sh

# with micod

MICORC=/dev/null
export MICORC

ADDR=inet:`uname -n`:17657

micod -ORBIIOPAddr $ADDR &
pid=$!

trap "kill $pid" 0
sleep 1

imr create StringSender poa ./server/Server IDL:StringSender:1.0 -ORBImplRepoAddr $ADDR
./client/Client $ADDR
