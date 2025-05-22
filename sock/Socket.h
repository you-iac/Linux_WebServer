//
// Created by ccccc on 25-5-21.
//

#include<string>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
using namespace std;

#ifndef SOCKET_H
#define SOCKET_H



class Socket {
protected:
    string ip_address;
    int port;

    Socket::Socket(const string& IP, int port);
    ~Socket();

public:
    bool CreateListren();
    bool read();
    bool close();





};



#endif //SOCKET_H
