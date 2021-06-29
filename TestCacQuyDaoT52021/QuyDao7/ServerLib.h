#ifndef __SERVERLIB_H__
#define __SERVERLIB_H__

#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>

#define PORT 8080

void ServerSocketInit();

class ServerSocket
{
public:
    int server_fd;
    int new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    ServerSocket();
    ~ServerSocket();
    void ServerInit();
    float* GetCoordinates();
    void start();
    int CheckStop();
    int SendData(const char *buffer);
};

#endif
