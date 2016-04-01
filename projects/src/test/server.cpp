#include "../util/testepoll.h"
#include "coro.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <map>

//extern int gs_epfd, gs_listenfd, gs_sockfd;
#define MAX_CLIENT 1024
#define IP "127.0.0.1"
#define PORT 8888


struct packet
{
    int cmd;
    char content[1024];
};

struct session
{
    int fd;
    int uid;
    char addr[32];
    int port;
    char custom_data[1024];
};

typedef int(*func)(struct packet pkg);

class EpollServer
{
private:
    int epollfd;
    int listenfd;
    std::map<int, func> handle_map;
public:
    void add_handler(int cmd, func f);
    int sock_bind(const char* ip, const int port);
    int handle_connection();
};


void EpollServer::add_handler(int cmd, func f)
{
    handle_map.insert(std::pair<int, func>(cmd,f));
}

int EpollServer::sock_bind(const char* ip, const int port)
{
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));

    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("sockfd\n");
        exit(1);
    }
    setnonblocking(listenfd);
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(ip);
    local.sin_port= htons(port);
    if(bind(listenfd, (struct sockaddr*)&local, sizeof(local)) < 0)
    {
        perror("bind\n");
        exit(1);
    }
    listen(listenfd, MAX_CLIENT);
    epollfd = epoll_create(20);
    
    if(epollfd == -1)
    {
        perror("epoll_create\n");
        exit(EXIT_FAILURE);
    }

    //不需要多个ev，因为都是拷贝到epoll内核的，每次wait到之后再拷贝回来，可以复用一个ev即可
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1)
    {
        perror("epoll_ctl\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}


int EpollServer::handle_connection()
{
    struct epoll_event ev[1024];
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    while(1)
    {
        int ret = epoll_wait(epollfd, ev, 1024, -1);
        for(int i = 0; i < ret; i++)
        {
            if(ev[i].data.fd == listenfd && (ev[i].events & EPOLLIN))
            {             
                struct sockaddr_in client_addr;
                memset(&client_addr, 0, sizeof(client_addr));
                socklen_t addr_len = sizeof(client_addr);
                int clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &addr_len);
                if(clientfd == -1)
                {
                    printf("accept error!\n");
                    return -1;
                }
                else 
                {
                    printf("accept a new client: %s:%d\n",inet_ntoa(client_addr.sin_addr),client_addr.sin_port);
                }

                ev[i].data.fd = clientfd;
                ev[i].events = EPOLLIN|EPOLLET|EPOLLERR|EPOLLRDHUP;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, ev[i].data.fd, &ev[i]);
                //为该链接创建一个session 保存链接信息 创建协程 加入map
                //创建协程
                Coro *hcoro =(Coro*) malloc(sizeof(Coro));
                hcoro->init(clientfd, CORO_STACK_SIZE);
                CoroMgr::getInstance().add_coro(clientfd, hcoro);
                
            }
            else if(ev[i].events & EPOLLIN)
            {
                printf("in EPOLLIN\n");
                
                int ret = recv(ev[i].data.fd, buf, sizeof(buf), 0);
                if(ret == 0)
                {
                    printf("client closed...\n");
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, ev[i].data.fd, &ev[i]);
                    break;
                }
                else
                {
                    printf("message recv: %s", buf);
                    //send(ev[i].data.fd, buf, sizeof(buf), 0);
                    ev[i].events = EPOLLIN | EPOLLET ;
                    epoll_ctl(epollfd, EPOLL_CTL_MOD, ev[i].data.fd, &ev[i]);
                }
                
                //resume协程，传入函数和数据，进行处理
                Coro *hcoro = CoroMgr::getInstance().get_coro(ev[i].data.fd);
                if(!hcoro)
                {
                    printf("hcoro null \n");
                    return -1;
                }

                //以后改成解析数据包，根据cmd寻找响应函数
                int cmd = atoi(buf);
                if(cmd != 1 && cmd != 2)
                {
                    printf("cmd error\n");
                    continue;
                }
                hcoro->setCmd(cmd);
                hcoro->resume();
                printf("back resume\n");
            }   
            else if(ev[i].events & EPOLLOUT)
            {
                printf("In EPOLLOUT\n");
                send(ev[i].data.fd, buf, sizeof(buf), 0);
                
                ev[i].events = EPOLLIN | EPOLLET ;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, ev[i].data.fd, &ev[i]);
            }
        }
    }
    return 0;
}

int default_handler(struct packet pkg)
{
    printf("cmd %d\n", pkg.cmd);
    printf("cmd %s\n", pkg.content);
    return 0;
}


ucontext_t uctx_main;

int main()
{    
    //初始化协程调度器
    CoroMgr &mgr = CoroMgr::getInstance();
    mgr.init();
    
    EpollServer srv;
    srv.sock_bind("127.0.0.1", 8888);
    srv.handle_connection();
    return 0;
}
