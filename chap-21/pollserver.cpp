#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

#define INIT_SIZE 128

int init_scoket(int port)
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));  // 地址重用

    int rt1 = bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(rt1 < 0)
    {
        error(1, errno, "bind failed");
    }

    int rt2 = listen(listenfd, LISTENQ);
    if(rt2 < 0)
    {
        error(1, errno, "listen failed");
    }

    signal(SIGPIPE, SIG_IGN);
    
    return listenfd;
}

int main(int argc, char* argv[])
{
    int listen_fd, conn_fd;
    int ready_number;
    ssize_t n;
    char buf[MAXLINE];
    struct sockaddr_in client_addr;

    listen_fd = init_scoket(SERV_PORT);

    // 初始化pollfd数组，这个数组的第一个元素是listen_fd，其余用来记录将要连接的connect_fd
    struct pollfd event_set[INIT_SIZE];
    event_set[0].fd = listen_fd;
    event_set[0].events = POLLRDNORM;

    // 用-1表示这个数组位置还没被占用
    for(int i = 1; i < INIT_SIZE; i++)
    {
        event_set[i].fd = -1;
    }

    for(;;)
    {
        if((ready_number = poll(event_set, INIT_SIZE, -1)) < 0)
        {
            error(1, errno, "poll failed");
        }

        if(event_set[0].revents & POLLRDNORM)
        {
            socklen_t client_len = sizeof(client_addr);
            conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);

            // 找到一个可以记录该连接套接字的位置
            int j;
            for(j = 1; j < INIT_SIZE; j++)
            {
                if(event_set[j].fd < 0 )
                {
                    event_set[j].fd = conn_fd;
                    event_set[j].events = POLLRDNORM;
                    break;
                }
            }

            if(j == INIT_SIZE)
            {
                error(1, errno, "can not hold so many clients");
            }

            if(--ready_number <= 0)
            {
                continue;
            }
        }

        for(int i = 1; i < INIT_SIZE; i++)
        {
            int socket_fd;
            if((socket_fd = event_set[i].fd) < 0)
            {
                continue;
            }
            if(event_set[i].revents & (POLLRDNORM | POLLERR))
            {
                if((n = read(socket_fd, buf, MAXLINE)) > 0)
                {
                    if(write(socket_fd, buf, n) < 0)
                    {
                        error(1, errno, "write error");
                    }
                }
                else if(n == 0 || errno == ECONNRESET)
                {
                    close(socket_fd);
                    event_set[i].fd = -1;
                }
                else
                {
                    error(1, errno, "read error");
                }

                if(--ready_number <= 0)  // 判断事件是否处理完
                    break;
            }
        }
    }

    exit(0);
}