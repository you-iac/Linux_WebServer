//2025 6 17 18 56 

//
// Created by 28695 on 2025/6/17.
//
#include "processpool.h"
/**
 * 设置文件描述符为非阻塞模式
 * @param fd 文件描述符
 * @return 原来的描述符
 */
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


/**
 * 进程池构造函数
 * @tparam T        模板
 * @param listenfd  监听描述符
 * @param process_number 进程数
 */
template< typename T >
processpool< T >::processpool( int listenfd, int process_number )
        : m_listenfd( listenfd ), m_process_number( process_number ), m_idx( -1 ), m_stop( false ){
    /**
     * 创建多个进程对象
     * 创建UNIX域套接字对为每个子进程创建双向管道
     * 创建子进程
     * 关闭父子进程一段实现通信
     */
    assert( ( process_number > 0 ) && ( process_number <= MAX_PROCESS_NUMBER ) );

    m_sub_process = new process[ process_number ];
    assert( m_sub_process );

    for( int i = 0; i < process_number; ++i )
    {
        /*创建UNIX域套接字对 为每个子进程创建双向管道*/
        int ret = socketpair( PF_UNIX, SOCK_STREAM, 0, m_sub_process[i].m_pipefd );
        assert( ret == 0 );

        /*创建子进程*/
        m_sub_process[i].m_pid = fork();
        assert( m_sub_process[i].m_pid >= 0 );

        /*判断父子进程，关闭管道的一端, 父进程继续循环，子进程提出*/
        if( m_sub_process[i].m_pid > 0 )
        {
            close( m_sub_process[i].m_pipefd[1] );
            continue;
        }
        else
        {
            close( m_sub_process[i].m_pipefd[0] );
            m_idx = i;
            break;
        }
    }
}
/**
 * 初始化全局管道，用于接受信号，并且注册信号
 * @tparam T 模板类
 */
template< typename T >
void processpool< T >::setup_sig_pipe()
{
    /**
     * 创建epoll文件描述符，
     * 创建全局管道，设置非阻塞，并且添加到epoll中
     * 注册信号处理
     */
    m_epollfd = epoll_create( 5 );
    assert( m_epollfd != -1 );

    int ret = socketpair( PF_UNIX, SOCK_STREAM, 0, sig_pipefd );
    assert( ret != -1 );

    setnonblocking( sig_pipefd[1] );
    addfd( m_epollfd, sig_pipefd[0] );

    addsig( SIGCHLD, sig_handler );
    addsig( SIGTERM, sig_handler );
    addsig( SIGINT, sig_handler );
    addsig( SIGPIPE, SIG_IGN );
}
/**
 * 根据线程id实现分别执行父/子进程代码
 * @tparam T 模板类
 */
template< typename T >
void processpool< T >::run()
{
    /*根据成员变量m_idx来判断当前是父进程还是子进程*/
    if( m_idx != -1 )
    {
        run_child();
        return;
    }
    /*运行父进程*/
    run_parent();
}
/**
 * 子进程运行逻辑
 * @tparam T
 */
template< typename T >
void processpool< T >::run_child()
{
    /*为子进程创建全局管道，用于子进程的接受信号，并且注册信号
     * 创建用户对象
     * 轮循事件
     * */
    setup_sig_pipe();

    int pipefd = m_sub_process[m_idx].m_pipefd[ 1 ];
    addfd( m_epollfd, pipefd );

    epoll_event events[ MAX_EVENT_NUMBER ];
    T* users = new T [ USER_PER_PROCESS ];
    assert( users );

    int number = 0;
    int ret = -1;

    while( ! m_stop )
    {
        /*使用epoll_wait轮循全部正在阻塞的事件*/
        number = epoll_wait( m_epollfd, events, MAX_EVENT_NUMBER, -1 );
        if ( ( number < 0 ) && ( errno != EINTR ) )
        {
            printf( "epoll failure\n" );
            break;
        }
        for ( int i = 0; i < number; i++ )
        {
            int sockfd = events[i].data.fd;
            /*父进程通过管道发送新连接通知。*/
            if( ( sockfd == pipefd ) && ( events[i].events & EPOLLIN ) )
            {
                int client = 0;
                ret = recv( sockfd, ( char* )&client, sizeof( client ), 0 );
                if( ( ( ret < 0 ) && ( errno != EAGAIN ) ) || ret == 0 )
                {
                    continue;
                }
                else
                {
                    /*调用 accept() 接收客户端连接*/
                    struct sockaddr_in client_address;
                    socklen_t client_addrlength = sizeof( client_address );
                    int connfd = accept( m_listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
                    if ( connfd < 0 )
                    {
                        printf( "errno is: %d\n", errno );
                        continue;
                    }
                    /*加入 epoll 监听*/
                    addfd( m_epollfd, connfd );
                    users[connfd].init( m_epollfd, connfd, client_address );
                }
            }
            /*io事件为管道有数据到达，说明有信号来临*/
            else if( ( sockfd == sig_pipefd[0] ) && ( events[i].events & EPOLLIN ) )
            {
                int sig;
                char signals[1024];
                ret = recv( sig_pipefd[0], signals, sizeof( signals ), 0 );
                if( ret <= 0 )
                {
                    continue;
                }
                else
                {
                    for( int i = 0; i < ret; ++i )
                    {
                        switch( signals[i] )
                        {
                            case SIGCHLD:/*子进程退出*/
                            {
                                pid_t pid;
                                int stat;
                                while ( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
                                {
                                    continue;
                                }
                                break;
                            }
                            case SIGTERM:/*终止请求*/
                            case SIGINT:/*Ctrl+C*/
                            {
                                m_stop = true;
                                break;
                            }
                            default:/*其他信号忽略*/
                            {
                                break;
                            }
                        }
                    }
                }
            }
            /*用户数据写入*/
            else if( events[i].events & EPOLLIN )
            {
                users[sockfd].process();
            }
            else
            {
                continue;
            }
        }
    }

    delete [] users;
    users = NULL;
    close( pipefd );
    //close( m_listenfd );
    close( m_epollfd );
}

template< typename T >
void processpool< T >::run_parent()
{
    setup_sig_pipe();

    addfd( m_epollfd, m_listenfd );

    epoll_event events[ MAX_EVENT_NUMBER ];
    int sub_process_counter = 0;
    int new_conn = 1;
    int number = 0;
    int ret = -1;

    while( ! m_stop )
    {
        number = epoll_wait( m_epollfd, events, MAX_EVENT_NUMBER, -1 );
        if ( ( number < 0 ) && ( errno != EINTR ) )
        {
            printf( "epoll failure\n" );
            break;
        }

        for ( int i = 0; i < number; i++ )
        {
            int sockfd = events[i].data.fd;
            /*新连接到达*/
            if( sockfd == m_listenfd )
            {
                int i =  sub_process_counter;
                do
                {
                    if( m_sub_process[i].m_pid != -1 )
                    {
                        break;
                    }
                    i = (i+1)%m_process_number;
                }
                while( i != sub_process_counter );

                if( m_sub_process[i].m_pid == -1 )
                {
                    m_stop = true;
                    break;
                }
                sub_process_counter = (i+1)%m_process_number;
                //send( m_sub_process[sub_process_counter++].m_pipefd[0], ( char* )&new_conn, sizeof( new_conn ), 0 );
                send( m_sub_process[i].m_pipefd[0], ( char* )&new_conn, sizeof( new_conn ), 0 );
                printf( "send request to child %d\n", i );
                //sub_process_counter %= m_process_number;
            }
            /*信号处理*/
            else if( ( sockfd == sig_pipefd[0] ) && ( events[i].events & EPOLLIN ) )
            {
                int sig;
                char signals[1024];
                ret = recv( sig_pipefd[0], signals, sizeof( signals ), 0 );
                if( ret <= 0 )
                {
                    continue;
                }
                else
                {
                    for( int i = 0; i < ret; ++i )
                    {
                        switch( signals[i] )
                        {
                            case SIGCHLD:
                            {
                                pid_t pid;
                                int stat;
                                while ( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
                                {
                                    for( int i = 0; i < m_process_number; ++i )
                                    {
                                        if( m_sub_process[i].m_pid == pid )
                                        {
                                            printf( "child %d join\n", i );
                                            close( m_sub_process[i].m_pipefd[0] );
                                            m_sub_process[i].m_pid = -1;
                                        }
                                    }
                                }
                                m_stop = true;
                                for( int i = 0; i < m_process_number; ++i )
                                {
                                    if( m_sub_process[i].m_pid != -1 )
                                    {
                                        m_stop = false;
                                    }
                                }
                                break;
                            }
                            case SIGTERM:
                            case SIGINT:
                            {
                                printf( "kill all the clild now\n" );
                                for( int i = 0; i < m_process_number; ++i )
                                {
                                    int pid = m_sub_process[i].m_pid;
                                    if( pid != -1 )
                                    {
                                        kill( pid, SIGTERM );
                                    }
                                }
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }
                    }
                }
            }

            else
            {
                continue;
            }
        }
    }

    //close( m_listenfd );
    close( m_epollfd );
}
