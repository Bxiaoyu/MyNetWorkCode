#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

/*
* 一般来说，服务器端不会主动发起 connect 操作，因为一旦如此，服务器端就只能响应一个客户端了。
* 不过，有时候也不排除这样的情形，一旦一个客户端和服务器端发送 UDP 报文之后，该服务器端就要服务于这个唯一的客户端。

* 在对 UDP 进行 connect 之后，关于收发函数的使用，很多书籍是这样推荐的：
    1.使用 send 或 write 函数来发送，如果使用 sendto 需要把相关的 to 地址信息置零；
    2.使用 recv 或 read 函数来接收，如果使用 recvfrom 需要把对应的 from 地址信息置零。
    其实不同的 UNIX 实现对此表现出来的行为不尽相同，考虑到兼容性，我们也推荐这些常规做法。
*/

static int count;

static void recvfrom_int(int signo)
{
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

int main(int argc, char* argv[])
{
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    socklen_t client_len;
    char message[MAXLINE];
    message[0] = 0;
    count = 0;

    signal(SIGINT, recvfrom_int);

    struct sockaddr_in client_addr;
    client_len = sizeof(client_addr);

    int n = recvfrom(socket_fd, message, MAXLINE, 0, (struct sockaddr*)&client_addr, &client_len);
    if (n < 0)
    {
        error(1, errno, "recvfrom failed!\n");
    }
    message[n] = 0;
    printf("received %d bytes: %s\n", n, message);

    if(connect(socket_fd, (struct sockaddr*)&client_addr, client_len))
    {
        error(1, errno, "connect failed!\n");
    }

    while(strncmp(message, "goodbye", 7) != 0)
    {
        char send_line[MAXLINE];
        sprintf(send_line, "Hi, %s", message);

        size_t rt = send(socket_fd, send_line, strlen(send_line), 0);
        if(rt < 0)
        {
            error(1, errno, "send failed!\n");
        }
        printf("send bytes: %zu \n", rt);

        size_t rc = recv(socket_fd, message, MAXLINE, 0);
        if (rc < 0)
        {
            error(1, errno, "recv failed!\n");
        }

        count++;
    }

    exit(0);
}