//
// Created by 28695 on 2025/5/22.
//
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
int main(int argc,char*argv[])
{
    if(argc<=2)
    {
        printf("usage:%s ip_address port_number\n",basename(argv[0]));
        return 1;
    }
    const char*ip=argv[1];
    int port=atoi(argv[2]);

    //创建地址结构
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port=htons(port);

    //创建scoket描述符
    int sock=socket(PF_INET,SOCK_STREAM,0);
    assert(sock>=0);

    //绑定端口
    int ret=bind(sock,(struct sockaddr*)&address,sizeof(address));
    assert(ret!=-1);

    //监听
    ret=listen(sock,5);
    assert(ret!=-1);

    //客户地址
    struct sockaddr_in client;
    socklen_t client_addrlength=sizeof(client);

    //接受连接
    int connfd=accept(sock,(struct sockaddr*)&client,&client_addrlength);

    if(connfd<0)
    {
        printf("errno is:%d\n",errno);
    }
    else
    {
        //关闭标准输出文件描述符 其值是1
        close(STDOUT_FILENO);

        //复制socket文件描述符connfd，connfd指向网络连接，
        dup(connfd);

        printf("abcd\n");
        close(connfd);
    }
    close(sock);
    return 0;
}
