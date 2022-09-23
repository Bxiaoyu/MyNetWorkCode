#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

/*
*   字节流本地套接字（服务器）
*/

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        error(1, 0, "usage: unixstreamserver <local_path>.\n");
    }

    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_un serveraddr, cliaddr;

    listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        error(1, errno, "socket create failed.\n");
    }

    char* local_path = argv[1];
    unlink(local_path);  // 我们这里还做了一个 unlink 操作，以便把存在的文件删除掉，这样可以保持幂等性。
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sun_family = AF_LOCAL;
    strcpy(serveraddr.sun_path, local_path);

    if(bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    {
        error(1, errno, "bind failed.\n");
    }

    if(listen(listenfd, LISTENQ) < 0)
    {
        error(1, errno, "listen failed.\n");
    }

    clilen = sizeof(cliaddr);
    if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) < 0)
    {
        if(errno == EINTR)
            error(1, errno, "accept failed.\n");
        else
            error(1, errno, "accept failed.\n");
    }

    char buf[BUFFER_SIZE];
    while (1)
    {
        bzero(buf, sizeof(buf));
        if(read(connfd, buf, BUFFER_SIZE) == 0)
        {
            printf("client quit.\n");
            break;
        }
        printf("Receive: %s", buf);

        char send_line[MAXLINE];
        bzero(send_line, MAXLINE);
        sprintf(send_line, "Hi, %s", buf);

        int nbytes = sizeof(send_line);

        if(write(connfd, send_line, nbytes) != nbytes)
        {
            error(1, errno, "write error.\n");
        }
    }

    close(listenfd);
    close(connfd);

    exit(0);
}