#include <iostream>

/*
*   测试close关闭和shutdown关闭的区别
*/

extern "C"
{
    #include "../lib/common.h"
}

static int count;

static void sig_int(int signo)
{
    printf("\nsig_int: received %d datagrams\n", count);
    exit(0);
}

static void sig_pipe(int signo)
{
    printf("\nsig_pipe: received %d datagrams\n", count);
    exit(0);
}


int main(int argc, char* argv[])
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

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
    signal(SIGPIPE, sig_pipe);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    if((connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len)) < 0)
    {
        error(1, errno, "accept failed!\n");
    }

    char message[MAXLINE];
    count = 0;

    for(;;)
    {
        int n = read(connfd, message, MAXLINE);
        if (n < 0)
        {
            error(1, errno, "error read!\n");
        }
        else if(n == 0)
        {
            error(1, 0, "client closed!\n");
        }
        message[n] = 0;
        printf("received %d bytes: %s\n", n, message);
        count++;

        char send_line[MAXLINE];
        sprintf(send_line, "Hi, %s", message);

        sleep(5);

        int write_nc = send(connfd, send_line, strlen(send_line), 0);
        printf("send bytes: %zu \n", write_nc);
        if (write_nc < 0)
        {
            error(1, errno, "error write!\n");
        }
    }

    return 0;
}