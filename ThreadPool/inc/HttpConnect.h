//
// Created by 28695 on 2025/6/20.
//

#ifndef MAIN_HTTPCONNECT_H
#define MAIN_HTTPCONNECT_H
#include "ThreadPool.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <iostream>
#include<fcntl.h> /**文件控制函数**/
#include<unistd.h>
#include <assert.h>
#include<arpa/inet.h>
#include <cstring>
/**
 * 服务器类，监听本地套接字，并且将工作逻辑交给线程池
 *
 */
class Connect{
    int _sockfd;
    static const int BUFFSIZE = 1024;
public:
    Connect(int sockfd):_sockfd(sockfd){}
    bool process(){
        std::cout << "接受端口的连接:" << this->_sockfd << std::endl;
        if (_sockfd >= 0 ) {
            char temp[BUFFSIZE];
            memset(temp,'\0',BUFFSIZE);

            int ret = 1;
            while (ret > 0 ) {
                ret = recv(_sockfd,temp,BUFFSIZE-1,0);
                std::cout << temp << "  titl " << ret<<std::endl;
            }
            close(_sockfd);
            return true;
        }else {
            std::cout << "error";
            return false;
        }

    }
};
class HttpConnect {
protected:
    const int THREAD_NUMBER = 8; /*工作线程数*/
    const int MAX_EVENT_NUMBER = 10000;
    int         _RUN = 1;
    int         _sockfd;
    ThreadPool<Connect> *_tp;
    static HttpConnect* _htpcnt; /*HttpConnect类单例指针*/
public:
    static HttpConnect* Create(int threadnumber ){
        if(_htpcnt){
           _htpcnt = new HttpConnect(threadnumber);
        }
        return HttpConnect::_htpcnt;
    }
    virtual ~HttpConnect();
    bool Run_OneToN();
    bool Run_N();

private:
    HttpConnect(int threadnumber);
    int CreatSocket(char* ip_address, int port);

};



#endif //MAIN_HTTPCONNECT_H
