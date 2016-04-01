#include "testepoll.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_CLIENT 1024

int gs_epfd, gs_listenfd, gs_sockfd;


void setnonblocking(int sockfd)
{
    int opts = fcntl(sockfd, F_GETFL);
    if(opts < 0)
    {
        perror("fcntl F_GETFL \n");
        exit(1);
    }

    opts = opts | O_NONBLOCK;
    if(fcntl(sockfd, F_SETFL, opts) < 0)
    {
        perror("fcntl F_SETFL \n");
        exit(1);
    }

    int iReuseaddr = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&iReuseaddr, sizeof(int));
}

void socket_bind(const char* addr, int port)
{
    int &epfd = gs_epfd;
    int &sockfd = gs_sockfd;
    int &listenfd = gs_listenfd;

    struct sockaddr_in local;
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("sockfd\n");
        exit(1);
    }
    setnonblocking(listenfd);
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(addr);
    local.sin_port= htons(port);
    if(bind(listenfd, (struct sockaddr*)&local, sizeof(local)) < 0)
    {
        perror("bind\n");
        exit(1);
    }
    listen(listenfd, MAX_CLIENT);
    epfd = epoll_create(20);
    if(epfd == -1)
    {
        perror("epoll_create\n");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) == -1)
    {
        perror("epoll_ctl\n");
        exit(EXIT_FAILURE);
    }

}

int epoll_read(int fd, char* buf, size_t buf_size)
{
    int buflen = 0;
    for(;;)
    {
        int readlen = recv(fd, buf+buflen, buf_size, 0);
        buflen += readlen;
        if(readlen < 0)
        {
            if(errno == EAGAIN || errno == EINTR)
                return 0;
            else
                return -1;
        }
        else if(readlen == 0)
        {
            break;
        }

        if(buflen == buf_size)
            continue;
        else
            break;
    }

    return buflen;
}



int epoll_write(int fd, const char* buffer, int buflen)
{
    int tmp;
    int total = buflen;
    const char *p = buffer;
    
    for(;;)
    {
        tmp = send(fd, p, total, 0);
        if(tmp < 0)
        {
            if(errno == EAGAIN)
            {
                usleep(1000);
                continue;
            }
            return -1;
        }

        if(tmp == total)
            return buflen;

        total -= tmp;
        p += tmp;
    }
}

void handle_event(int epfd, struct epoll_event *events, int num, int listenfd, char *buf )
{
    for(int i = 0; i < num; i ++)
    {
        int fd = events[i].data.fd;
        char buf[1024];
        memset(buf, 0, 1024);
        if((fd == listenfd) && (events[i].events & EPOLLIN))
            handle_accept(epfd, fd);
        else if(events[i].events & EPOLLIN)
        {
            printf("message recved\n");
            int ret = epoll_read(fd, buf, sizeof(buf));
            if(ret == 0)
            {
                printf("%s\n", buf);
            }
            else
            {
                printf("read null\n");
            }

            //modify_event(epfd, fd, EPOLLIN | EPOLLET);
            //delete_event(epfd, fd, EPOLLIN);
        }
        else if(events[i].events & EPOLLOUT)
        {
            printf("message write");
           // modify_event(epfd, fd, EPOLLIN);
        }
    }
}


void handle_accept(int epfd, int listenfd)
{
    int clifd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    clifd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
    if(clifd == -1)
        perror("accept error\n");
    else
    {
        printf("accept a new client: %s:%d\n",inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port);
        //add_event(epfd, clifd, EPOLLIN | EPOLLET);
    }
}

void add_event(int epfd, int fd, int state, struct epoll_event& ev)
{
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

void modify_event(int epfd, int fd, int state, struct epoll_event& ev)
{
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}

void delete_event(int epfd, int fd, int state, struct epoll_event& ev)
{
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
}

































