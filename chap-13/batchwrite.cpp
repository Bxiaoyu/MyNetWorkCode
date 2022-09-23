#include <iostream>

/**
 * 合并写操作(writev(...))，避免Nagle算法(解决小数据包浪费网络带宽的算法)带来的副作用
**/

extern "C"
{
    #include "../lib/common.h"
}

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
    if(connect_rt < 0)
    {
        error(1, errno, "connect failed!\n");
    }

    char buf[128];
    struct iovec iov[2];

    char* send_one = "hello,";
    iov[0].iov_base = send_one;
    iov[0].iov_len = strlen(send_one);
    iov[1].iov_base = buf;
    while(fgets(buf, sizeof(buf), stdin) != NULL)
    {
        iov[1].iov_len = strlen(buf);
        int n = htonl(iov[1].iov_len);
        if(writev(socket_fd, iov, 2) < 0)
        {
            error(1, errno, "writev failure!\n");
        }
    }

    exit(0);
}
