#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>


int main()
{
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    char buf[1024];
    while(1)
    {
        memset(buf, 0, sizeof(buf));
        int ret = read(0, buf, sizeof(buf));
        if(buf[0] == 'q')
            break;
        send(sockfd, buf, sizeof(buf), 0);
        printf("sent %s",buf);

       //memset(buf, 0, sizeof(buf));
        //recv(sockfd, buf, sizeof(buf), 0);
        //printf("recv %s",buf);
        
    }

    close(sockfd);

    return 0;
}

