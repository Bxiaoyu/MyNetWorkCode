#include <iostream>
#include "message_object.h"

/*
*   在应用层面模拟TCP保活机制（客户端）
*/

extern "C"
{
    #include "../lib/common.h"
}

#define MAXLINE 4096
#define KEEP_ALIVE_TIME 10       // 保活时间
#define KEEP_ALIVE_INTERVAL 3    // 保活时间间隔
#define KEEP_ALIVE_PROBETIMES 3  // 保活探测次数

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        error(1, 0, "usage: tcpclient <IPaddress>.\n");
    }

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    int connect_rt = connect(socket_fd, (struct sockaddr*)&server_addr, server_len);
    if (connect_rt < 0)
    {
        error(1, errno, "connect failed!\n");
    }

    char recv_line[MAXLINE + 1];
    int n;

    fd_set readmask;
    fd_set allreads;

    struct timeval tv;  // 定时器
    int heartbeats = 0;

    tv.tv_sec = KEEP_ALIVE_TIME;
    tv.tv_usec = 0;

    messageObject mbj;

    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);

    for(;;)
    {
        readmask = allreads;
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, &tv);
        if (rc < 0)
        {
            error(1, errno, "select failed!\n");
        }
        if (rc == 0)
        {
            if(++heartbeats > KEEP_ALIVE_PROBETIMES)
            {
                error(1, errno, "connection dead!\n");
            }
            printf("sending heartbeat #%d\n", heartbeats);
            mbj.type = htonl(MSG_PING);
            rc = send(socket_fd, (char*)&mbj, sizeof(mbj), 0);
            if (rc < 0)
            {
                error(1, errno, "send failed!\n");
            }
            tv.tv_sec = KEEP_ALIVE_INTERVAL;
            continue;
        }
        if(FD_ISSET(socket_fd, &readmask))
        {
            n = read(socket_fd, recv_line, MAXLINE);
            if (n < 0)
            {
                error(1, errno, "read error!\n");
            }
            else if (n == 0)
            {
                error(1, 0, "server terminated!\n");
            }
            printf("received heartbeat, make heartbeats to 0.\n");
            heartbeats = 0;
            tv.tv_sec = KEEP_ALIVE_TIME;
        }
    }

    return 0;
}