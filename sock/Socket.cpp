//
// Created by ccccc on 25-5-21.
//

#include "../Socket.h"

Socket::Socket(const string& IP, int port) {
    this->ip_address = IP;
    this->port = port;
}

Socket::~Socket() {

}

bool Socket::CreateListren() {


    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr =  inet_addr(this->ip_address.c_str());
    address.sin_port = htons(this->port);
    //
    int sock = socket(PF_INET,SOCK_STREAM,0);

    int ret = bind(sock, (struct sockaddr *)&address, sizeof(address));
    assert(ret!=-1);
    ret = listen(sock,5);
    assert(ret!=-1);



}

bool Socket::read() {
}

bool Socket::close() {
}
