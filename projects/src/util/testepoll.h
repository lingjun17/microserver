#ifndef __TESTEPOLL_H__
#define __TESTEPOLL_H__

void setnonblocking(int sockfd);

void socket_bind(const char* addr, int port);

int epoll_read(int fd, char* buf, int buf_size);

int epoll_write(int fd, const char* buffer, int buflen);

void handle_accept(int epfd, int listenfd);

void add_event(int epfd, int fd, int state, struct epoll_event &ev);

void modify_event(int epfd, int fd, int state, struct epoll_event &ev);

void delete_event(int epfd, int fd, int state, struct epoll_event &ev);

void handle_event(int epfd, struct epoll_event *events, int num, int listenfd, char *buf );

#endif
