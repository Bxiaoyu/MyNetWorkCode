#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

/*
    测试边缘触发
*/

#define MAXEVENTS 128

void make_nonblock(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int tcp_nonblocking_serve_listen2(int port)
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    make_nonblock(listenfd);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // 对TCP套接字处于TIME_WAIT状态下的socket，用SO_REUSEADDR参数设置重复绑定使用（重用套接字）
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

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

    // 为了避免进程退出，可以捕获 SIGPIPE 信号，或者忽略它，给它设置 SIG_IGN 信号
    signal(SIGPIPE, SIG_IGN);

    return listenfd;
}

int main(int argc, char* argv[])
{
    int listen_fd, socket_fd;
    int n;
    int efd;
    struct epoll_event event;
    struct epoll_event* events;

    listen_fd = tcp_nonblocking_serve_listen2(SERV_PORT);

    efd = epoll_create1(0);
    if(efd == -1)
    {
        error(1, errno, "epoll create failed");
    }

    event.data.fd = listen_fd;
    event.events = POLLIN | EPOLLET;  // 边缘触发
    if(epoll_ctl(efd, EPOLL_CTL_ADD, listen_fd, &event) == -1)
    {
        error(1, errno, "epoll_ctl add listen fd failed");
    }

    /* Buffer where events are returned */
    events = new epoll_event[MAXEVENTS];

    while(1)
    {
        n = epoll_wait(efd, events, MAXEVENTS, -1);
        printf("epoll_wait wakeup\n");
        for(int i = 0; i < n; i++)
        {
            // 判断异常情况并处理
            if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
            {
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            else if(listen_fd == events[i].data.fd)
            {
                struct sockaddr_storage ss;
                socklen_t slen = sizeof(ss);

                int fd = accept(listen_fd, (struct sockaddr*)&ss, &slen);
                if(fd < 0)
                {
                    error(1, errno, "accept failed");
                }
                else
                {
                    make_nonblock(fd);
                    event.data.fd = fd;
                    event.events = EPOLLIN | EPOLLET;
                    if(epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1)
                    {
                        error(1, errno, "epoll_ctl add connection fd failed");
                    }
                }
                continue;
            }
            else
            {
                socket_fd = events[i].data.fd;
                printf("get event on socket fd == %d \n", socket_fd);
            }
        }
    }

    delete[] events;
    close(listen_fd);
}