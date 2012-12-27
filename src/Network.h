#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>

class Network
{

    pthread_t recvThread;
    pthread_t sendThread;

    static void* recvThreadMain(void* ptr);
    static void* sendThreadMain(void* ptr);

public:
    Network(int port, const char* remoteAddr);
};

#endif // NETWORK_H
