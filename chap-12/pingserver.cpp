#include <iostream>
#include "message_object.h"

/*
*   在应用层面模拟TCP保活机制（服务端）
*/

extern "C"
{
    #include "../lib/common.h"
}

static int count;

static void sig_int(int signo)
{
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        error(1, 0, "usage: tcpserver <sleepingtime>.\n");
    }

    int sleepingtime = atoi(argv[1]);

    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    int rt1 = bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rt1 < 0)
    {
        error(1, errno, "bind failed!\n");
    }

    int rt2 = listen(listenfd, LISTENQ);
    if (rt2 < 0)
    {
        error(1, errno, "listen failed!\n");
    }

    signal(SIGINT, sig_int);
    signal(SIGPIPE, SIG_IGN);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    if((connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len)) < 0)
    {
        error(1, errno, "bind failed.\n");
    }

    messageObject mbj;
    count = 0;

    for (;;)
    {
        int n = read(connfd, (char*)&mbj, sizeof(messageObject));
        if(n < 0)
        {
            error(1, errno, "error read!\n");
        }
        else if(n == 0)
        {
            error(1, 0, "client closed!\n");
        }

        printf("received %d bytes\n", n);
        count++;

        switch (ntohl(mbj.type))
        {
        case MSG_TYPE1:
            printf("process MSG_TYPE1 \n");
            break;
        case MSG_TYPE2:
            printf("process MSG_TYPE2 \n");
            break;
        case MSG_PING:
        {
            messageObject pong_message;
            pong_message.type = htonl(MSG_PONG);
            sleep(sleepingtime);
            ssize_t rc = send(connfd, (char*)&pong_message, sizeof(pong_message), 0);
            if (rc < 0)
            {
                error(1, errno, "send failed!\n");
            }
            break;
        }
        default:
            error(1, 0, "unknown message type (%d)\n", ntohl(mbj.type));
        }
    }

    return 0;
}