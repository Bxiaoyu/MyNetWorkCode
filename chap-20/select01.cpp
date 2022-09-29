#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

/*
*   I/O多路复用：select
*   缺点：所支持的文件描述符个数有限，在Linux中select默认最大值为1024
*   描述符操作宏：
*       FD_ZERO 用来将这个向量的所有元素都设置成 0；
*       FD_SET 用来把对应套接字 fd 的元素，a[fd]设置成 1；
*       FD_CLR 用来把对应套接字 fd 的元素，a[fd]设置成 0；
*       FD_ISSET 对这个向量进行检测，判断出对应套接字的元素 a[fd]是 0 还是 1。
*/

#define MAXLINE 1024

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        error(1, 0, "usgae: select01 <IPaddress>");
    }

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    
    int connect_rt = connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(connect_rt < 0)
    {
        error(1, errno, "connect failed");
    }

    char send_line[MAXLINE], recv_line[MAXLINE];
    int n;

    fd_set readmask;
    fd_set allreads;
    FD_ZERO(&allreads);            // 初始化描述符集合，空集合
    FD_SET(0, &allreads);          // 设置fd=0处描述符为待检测(1)
    FD_SET(socket_fd, &allreads);  // 设置fd=socket_fd处描述符为待检测(1)

    for(;;)
    {
        readmask = allreads;  // 因为select每次调用后内核都会修改描述符集合，所以每次测试完都要重置描述符集合，保证前面设置的几个描述符能被一直监测
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, NULL);

        if(rc < 0)
        {
            error(1, errno, "select failed");
        }

        if(FD_ISSET(socket_fd, &readmask))
        {
            n = read(socket_fd, recv_line, MAXLINE);
            if(n < 0)
            {
                error(1, errno, "read error");
            }
            else if(n == 0)
            {
                error(1, 0, "server terminated \n");
            }

            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }

        if(FD_ISSET(STDIN_FILENO, &readmask))
        {
            if(fgets(send_line, MAXLINE, stdin) != NULL)
            {
                int i = strlen(send_line);
                if(send_line[i - 1] == '\n')
                {
                    send_line[i - 1] = 0;
                }

                printf("now sending %s\n", send_line);
                ssize_t rt = write(socket_fd, send_line, strlen(send_line));
                if(rt < 0)
                {
                    error(1, errno, "write failed ");
                }
                printf("send bytes: %zu\n", rt);
            }
        }
    }

    exit(0);
}