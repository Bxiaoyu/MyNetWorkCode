#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

static int count;

static void recvfrom_int(int signo)
{
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}


int main(int argc, char* argv[])
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    socklen_t client_len;
    char message[MAXLINE];
    count = 0;

    signal(SIGINT, recvfrom_int);

    struct sockaddr_in client_addr;
    client_len = sizeof(client_addr);
    for (;;)
    {
        int n = recvfrom(sockfd, message, MAXLINE, 0, (struct sockaddr*)&client_addr, &client_len);
        message[n] = 0;  // strlen函数以'\0'为结束符，字符串在没有初始化为0的状态下需要手动添加'\0'
        printf("received %d bytes: %s\n", n, message);

        char send_line[MAXLINE];
        sprintf(send_line, "Hi, %s", message);

        sendto(sockfd, send_line, strlen(send_line), 0, (struct sockaddr*)&client_addr, client_len);

        count++;
    }

    return 0;
}