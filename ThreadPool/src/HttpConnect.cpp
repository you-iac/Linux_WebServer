//
// Created by 28695 on 2025/6/20.
//

#include "../inc/HttpConnect.h"

HttpConnect* HttpConnect::_htpcnt = nullptr;

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


HttpConnect::~HttpConnect() {
    if(this->_sockfd > 0){
        close(_sockfd);
    }

    delete this->_tp;
}

HttpConnect::HttpConnect(int threadnumber) : THREAD_NUMBER(threadnumber)
{

    this->_sockfd = CreatSocket("127.0.0.1", 12345);

    /*创建sock描述符*/
    this->_tp = new ThreadPool<Connect>(this->THREAD_NUMBER,10000);


}

int HttpConnect::CreatSocket(char *ip_address = "127.0.0.1", int port= 12345) {
    /*//目的地址*/
    struct sockaddr_in address;
    bzero(&address,sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip_address);
    address.sin_port = htons(port);

    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    assert(sockfd >= 0);

    std::cout << "sockfd:" << sockfd <<std::endl;
    int ret = 0;
    ret = bind(sockfd,(struct sockaddr *)&address,sizeof(address));
    assert(ret >= 0);
    std::cout << "bind success:"  <<std::endl;

//    ret = listen(sockfd,5);
//    assert(ret >= 0);
//    cout << "listen success:"  <<endl;
//
//    struct sockaddr_in client;
//    bzero(&client,sizeof(client));
//
//    socklen_t len = sizeof(client);
//    int connfd = accept(sockfd,(struct sockaddr*)&client,&len);
//    assert(connfd >= 0);
//    cout << "connfd:" << connfd <<endl;
//    cout << "目的IP:" << inet_ntoa(client.sin_addr) << "port :" << ntohs(client.sin_port) <<endl;
//
//
//    if (connfd >= 0 ) {
//        char temp[BUFFSIZE];
//        memset(temp,'\0',BUFFSIZE);
//
//        int ret = 1;
//        while (ret > 0 ) {
//            ret = recv(connfd,temp,BUFFSIZE-1,0);
//            cout << temp << "  titl " << ret<<endl;
//        }
//        close(connfd);
//
//    }else {
//        cout << "error";
//    }


    return sockfd;
}

bool HttpConnect::Run_OneToN() {
    /*设置为非阻塞*/
    int ret = setnonblocking(this->_sockfd);
    /*添加到epoll*/
    int epfd = epoll_create(0);
    addfd(epfd,this->_sockfd);
    epoll_event events[ MAX_EVENT_NUMBER ];

    ret = listen(this->_sockfd,5);
    assert(ret >= 0);
    std::cout << "listen success:"  << std::endl;


    int envents_number = 0;
    while(!this->_RUN){
        envents_number = epoll_wait(epfd,events,MAX_EVENT_NUMBER,-1);
        for(int i = 0; i < MAX_EVENT_NUMBER; ++i)
        {
            int sockfd = events[i].data.fd;
            /*新连接到达*/
            if( sockfd == this->_sockfd )
            {
                /*创建用户结构体*/
                struct sockaddr_in client;
                bzero(&client,sizeof(client));
                /*接受连接*/
                socklen_t len = sizeof(client);
                int connfd = accept(sockfd,(struct sockaddr*)&client,&len);
                assert(connfd >= 0);
                /*添加到线程工作队列*/
                Connect* user = new Connect (connfd);
                this->_tp->append(user);
            }
        }
    }
    return false;
}



///**
// * 信号处理（接收）函数 将接收到的信号通过管道发送给主进程。
// * @param sig 触发的信号编号
// */
//static void sig_handler( int sig )
//{
//
//    int save_errno = errno;
//    int msg = sig;
//    send( sig_pipefd[1], ( char* )&msg, 1, 0 );
//    errno = save_errno;
//}
//
///**
// * 注册信号的信号处理函数
// * @param sig       信号
// * @param handler   处理函数的函数指针
// * @param restart
// */
//static void addsig( int sig, void( handler )(int), bool restart = true )
//{
//
//    struct sigaction sa;
//    memset( &sa, '\0', sizeof( sa ) );
//    sa.sa_handler = handler;
//    if( restart )
//    {
//        sa.sa_flags |= SA_RESTART;
//    }
//    sigfillset( &sa.sa_mask );
//    assert( sigaction( sig, &sa, NULL ) != -1 );
//}