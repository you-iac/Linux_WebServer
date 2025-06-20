//
// Created by 28695 on 2025/6/20.
//

#ifndef MAIN_MYTHREADPOOL_H
#define MAIN_MYTHREADPOOL_H

#include<pthread.h>
#include <signal.h>
#include <sys/epoll.h>

class Thread
{
public:
    Thread() : _tid( -1 ){}
public:
    pid_t _tid;
    int _epollfd;
};



template <typename T>
class ThreadPoolExecutor{
protected:
    static const int MAX_Thread_Number = 16; //最大线程数
    static const int USER_PER_PROCESS = 65536;//最大用户结构数
    static const int MAX_EVENT_NUMBER = 10000;//最大事件数
    int _thread_number;   /*进程数*/
    int _idx;              /*当前进程标识，判断是否为父进程*/
    int _epollfd;          /*epoll文件描述符*/
    int _listenfd;         /*监听描述符*/
    int _stop;             /*是否结束*/
    Thread* _sub_thread;
    static ThreadPoolExecutor< T >* _instance;

private:

    ThreadPoolExecutor(int listenfd, int threadNumber);

    ~ThreadPoolExecutor(){}

public:
    static ThreadPoolExecutor<T> * Create(int listenfd, int process_number = 8)
    {
        if( !_instance )
        {
            _instance = new ThreadPoolExecutor< T >( listenfd, process_number );
        }
        return _instance;
    }

};
/**
 *
 * @tparam T
 * @param listenfd 监听描述符
 * @param threadNumber 线程数
 */
template<typename T>
ThreadPoolExecutor<T>::ThreadPoolExecutor(int listenfd, int threadNumber)
    : _listenfd(listenfd), _thread_number(threadNumber)
{
    /**
     * 创建多个进程对象
     * 创建UNIX域套接字对为每个子进程创建双向管道
     * 创建子进程
     * 关闭父子进程一段实现通信
     */

    int ret = 0, tid = 0;
    ret = pthread_create(tid,,T->run(),);

}













static int setnonblocking( int fd )
{
    /*fcntl: 描述符控制函数，F_GETFL为获取状态标志*/
    int old_option = fcntl( fd, F_GETFL );   /*获取描述符的状态标志*/
    int new_option = old_option | O_NONBLOCK;/*创建新描述符，添加非阻塞*/

    /*fcntl: 描述符控制函数，F_SETFL为设置状态标志*/
    fcntl( fd, F_SETFL, new_option );/*设置新的描述符状态到描述符*/
    return old_option;
}
/**
 * 添加文件描述符到epoll监听
 * @param epollfd epollfd的描述符
 * @param fd 文件描述符
 */
static void addfd( int epollfd, int fd )
{
    /*创建一个epoll事件结构体，设置结构体fd，事件类型，
    使用epoll_ctl控制添加到epollfd，EPOLL_CTL_ADD为控制符意思是添加*/
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}
/**
 * 从epoll中移除文件描述符
 * @param epollfd epollfd的描述符
 * @param fd 文件描述符
 */
static void removefd( int epollfd, int fd )
{
    /*epoll_ctl为epoll控制函数，EPOLL_CTL_DEL为删除控制符*/
    epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
    close( fd );
}
/**
 * 信号处理（接收）函数 将接收到的信号通过管道发送给主进程。
 * @param sig 触发的信号编号
 */
static void sig_handler( int sig )
{

    int save_errno = errno;
    int msg = sig;
    send( sig_pipefd[1], ( char* )&msg, 1, 0 );
    errno = save_errno;
}

/**
 * 注册信号的信号处理函数
 * @param sig       信号
 * @param handler   处理函数的函数指针
 * @param restart
 */
static void addsig( int sig, void( handler )(int), bool restart = true )
{

    struct sigaction sa;
    memset( &sa, '\0', sizeof( sa ) );
    sa.sa_handler = handler;
    if( restart )
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset( &sa.sa_mask );
    assert( sigaction( sig, &sa, NULL ) != -1 );
}
#endif //MAIN_MYTHREADPOOL_H




