//
// Created by 28695 on 2025/6/10.
//
#ifndef PROCESSPOOL_H
#define PROCESSPOOL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

class process
{
public:
    process() : m_pid( -1 ){}

public:
    pid_t m_pid;
    int m_pipefd[2];
};

template< typename T >
class processpool
{
private:
    processpool( int listenfd, int process_number = 8 );
public:
    static processpool< T >* create( int listenfd, int process_number = 8 )
    {
        if( !m_instance )
        {
            m_instance = new processpool< T >( listenfd, process_number );
        }
        return m_instance;
    }
    ~processpool()
    {
        delete [] m_sub_process;
    }
    void run();

private:
    void setup_sig_pipe();
    void run_parent();
    void run_child();

private:
    static const int MAX_PROCESS_NUMBER = 16; //最大进程数
    static const int USER_PER_PROCESS = 65536;//最大用户结构数
    static const int MAX_EVENT_NUMBER = 10000;//最大事件数
    int m_process_number;   /*进程数*/
    int m_idx;              /*当前进程标识，判断是否为父进程*/
    int m_epollfd;          /*epoll文件描述符*/
    int m_listenfd;         /*监听描述符*/
    int m_stop;             /*是否结束*/
    process* m_sub_process;
    static processpool< T >* m_instance;
};

template< typename T >
processpool< T >* processpool< T >::m_instance = NULL;
static int sig_pipefd[2];

//class epoll{
//    static
//};


#endif
