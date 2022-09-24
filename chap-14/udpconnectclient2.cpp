#include <iostream>

/*
*   测试已连接的udp
* 原因：
    之所以对 UDP 使用 connect，绑定本地地址和端口，是为了让我们的程序可以快速获取异步错误信息的通知，同时也可以获得一定性能上的提升。
* 
* 报文发送方式：
    1.不使用connect：连接套接字→发送报文→断开套接字→连接套接字→发送报文→断开套接字 →………
    2.使用connect: 连接套接字→发送报文→发送报文→……→最后断开套接字
    连接套接字是需要一定开销的，比如需要查找路由表信息。所以，UDP 客户端程序通过 connect 可以获得一定的性能提升。
*/

extern "C"
{
    #include "../lib/common.h"
}

#define MAXLINE 4096


int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        error(1, 0, "usage: udpclient <IPaddress>\n");
    }

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);

    if(connect(socket_fd, (struct sockaddr*)&server_addr, server_len))
    {
        error(1, errno, "connect failed!\n");
    }

    char send_line[MAXLINE], recv_line[MAXLINE + 1];
    int n;

    while(fgets(send_line, MAXLINE, stdin) != NULL)
    {
        int i = strlen(send_line);
        if(send_line[i-1] == '\n')
        {
            send_line[i-1] = 0;
        }
        printf("now sending %s\n", send_line);
        size_t rt = send(socket_fd, send_line, strlen(send_line), 0);
        if(rt < 0)
        {
            error(1, errno, "send failed!\n");
        }
        printf("send bytes: %zu \n", rt);

        recv_line[0] = 0;
        n = recv(socket_fd, recv_line, MAXLINE, 0);
        if(n < 0)
        {
            error(1, errno, "recv failed!\n");
        }

        recv_line[n] = 0;
        fputs(recv_line, stdout);
        fputs("\n", stdout);
    }

    exit(0);
}