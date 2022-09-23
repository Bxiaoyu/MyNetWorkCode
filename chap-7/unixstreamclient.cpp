#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

/*
*   字节流本地套接字（客户端）
*/

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        error(1, 0, "usage: unixstreamclient <local_path>.\n");
    }

    int sockfd;
    struct sockaddr_un servaddr;

    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, argv[1]);

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        error(1, errno, "connect failed.\n");
    }

    char send_line[MAXLINE];
    bzero(send_line, MAXLINE);
    char recv_line[MAXLINE];

    while(fgets(send_line, MAXLINE, stdin) != NULL)
    {
        int nbytes = sizeof(send_line);
        if(write(sockfd, send_line, nbytes) != nbytes)
        {
            error(1, errno, "write error.\n");
        }

        if(read(sockfd, recv_line, sizeof(recv_line)) == 0)
        {
            error(1, errno, "server terminated prematurely.\n");
        }

        fputs(recv_line, stdout);
    }

    exit(0);
}