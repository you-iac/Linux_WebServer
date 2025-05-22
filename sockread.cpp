//
// Created by ccccc on 25-5-22.
//
#include <assert.h>
#include <cstring>
#include<iostream>
#include <unistd.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

using namespace std;
#define BUFFSIZE 1024


int main() {
    char ip_address[] = "192.168.90.129";
    int port = 12345;

    //目的地址
    struct sockaddr_in address;
    bzero(&address,sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip_address);
    address.sin_port = htons(port);


    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    assert(sockfd >= 0);
    cout << "sockfd:" << sockfd <<endl;

    int ret = 0;
    ret = bind(sockfd,(struct sockaddr *)&address,sizeof(address));
    assert(ret >= 0);
    cout << "bind success:"  <<endl;

    ret = listen(sockfd,5);
    assert(ret >= 0);
    cout << "listen success:"  <<endl;

    struct sockaddr_in client;
    bzero(&client,sizeof(client));

    socklen_t len = sizeof(client);
    int connfd = accept(sockfd,(struct sockaddr*)&client,&len);
    assert(connfd >= 0);
    cout << "connfd:" << connfd <<endl;
    cout << "目的IP:" << inet_ntoa(client.sin_addr) << "port :" << ntohs(client.sin_port) <<endl;


    if (connfd >= 0 ) {
        char temp[BUFFSIZE];
        memset(temp,'\0',BUFFSIZE);

        int ret = 1;
        while (ret > 0 ) {
            ret = recv(connfd,temp,BUFFSIZE-1,0);
            cout << temp << "  titl " << ret<<endl;
        }
        close(connfd);

    }else {
        cout << "error";
    }




}

