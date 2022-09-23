#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

#define NDG     2000  /* datagrams to send */
#define DGLEN   1400
#define MAXLINE 4096

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        error(1, 0, "usage: udpclient <IPaddress>.\n");
    }

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);

    struct sockaddr *reply_addr;
    reply_addr = new sockaddr;
    if(reply_addr == nullptr)
    {
        error(1, errno, "malloc reply_addr failed.\n");
    }

    char send_line[MAXLINE], recv_line[MAXLINE];
    socklen_t len;
    int n;

    while (fgets(send_line, MAXLINE, stdin) != NULL)  // fgets函数会自动给字符串加上'\0'
    {
        int i = strlen(send_line);
        if(send_line[i - 1] == '\n')
        {
            send_line[i - 1] = 0;
        }

        printf("now sending %s\n", send_line);
        size_t rt = sendto(socket_fd, send_line, strlen(send_line), 0, (struct sockaddr*)&server_addr, server_len);
        if(rt < 0)
        {
            error(1, errno, "send failed.\n");
        }
        printf("send bytes: %zu \n", rt);

        len = 0;
        n = recvfrom(socket_fd, recv_line, MAXLINE, 0, reply_addr, &len);
        if(n < 0)
        {
            error(1, errno, "recvfrom failed.\n");
        }
        recv_line[n] = 0;
        fputs(recv_line, stdout);
        fputs("\n", stdout);
    }

    exit(0);
}