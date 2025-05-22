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
    struct sockaddr_in servers;
    bzero(&servers,sizeof(servers));

    servers.sin_family = AF_INET;
    servers.sin_addr.s_addr = inet_addr(ip_address);
    servers.sin_port = htons(port);


    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    assert(sockfd >= 0);
    cout << "sockfd:" << sockfd <<endl;

    int ret = 0;
    // ret = bind(sockfd,(struct sockaddr *)&address,sizeof(address));
    // assert(ret >= 0);
    // cout << "bind success:"  <<endl;
    //



    ret = connect(sockfd,(struct sockaddr*)&servers,sizeof(servers));
    assert(ret >= 0);

    cout << "connfd:" << sockfd <<endl;
    cout << "目的IP:" << inet_ntoa(servers.sin_addr) << "port :" << ntohs(servers.sin_port) <<endl;

    struct sockaddr_in self;
    bzero(&self,sizeof(self));
    socklen_t len = sizeof(self);
    ret = getsockname(sockfd,(struct sockaddr*)&self,&len);

    cout << "selfIP:" << inet_ntoa(self.sin_addr) << "port :" << ntohs(self.sin_port) <<endl;



       char temp[BUFFSIZE];
        memset(temp,'\0',BUFFSIZE);
        cout << "请输入你的消息";
        cin >> temp;
        cout << "你要发的是：" << temp << endl;

        ret = send(sockfd,temp,BUFFSIZE-1,0);
        cout << "send success!!" << ret <<endl;

        close(sockfd);


}

