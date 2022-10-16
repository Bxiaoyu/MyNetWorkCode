#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

#define MAXEVENTS 128

char rot13_char(char c)
{
    if((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
    {
        return c + 13;
    }
    else if((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
    {
        return c - 13;
    }
    else
    {
        return c;
    }
}

// 调用 fcntl 将监听套接字设置为非阻塞
void make_nonblock(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int tcp_nonblocking_server_listen1(int port)
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    make_nonblock(listenfd);  // 设置监听套接字为非阻塞

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

    listen_fd = tcp_nonblocking_server_listen1(SERV_PORT);

    // epoll_create1() 的用法和 epoll_create() 基本一致，如果 epoll_create1() 的输入 flags 为 0，则和 epoll_create() 一样，内核自动忽略。
    // 从 Linux 2.6.8 开始，参数 size 被自动忽略
    efd = epoll_create1(0);
    if(efd == -1)
    {
        error(1, errno, "epoll create failed");
    }

    event.data.fd = listen_fd;
    event.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(efd, EPOLL_CTL_ADD, listen_fd, &event) == -1)
    {
        error(1, errno, "epoll_ctl add listen fd failed");
    }

    /*Buffer where events are returned*/
    events = new epoll_event[MAXEVENTS];

    while(1)
    {
        // int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
        // 返回值: 成功返回的是一个大于0的数，表示事件的个数；返回0表示的是超时时间到；若出错返回-1.
        n = epoll_wait(efd, events, MAXEVENTS, -1);  // timeout设为-1表示不超时，设置为0表示立即返回，即使没有任何I/O事件发生
        printf("epoll_wait wakeup\n");

        for(int i = 0; i < n; i++)
        {
            if((events[i].events & EPOLLERR) || 
               (events[i].events & EPOLLHUP) ||
               (!(events[i].events & EPOLLIN)))
               {
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
               }
               else if (listen_fd == events[i].data.fd)
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
                    event.events = EPOLLIN | EPOLLET;  //edge-triggered
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
                while(1)
                {
                    char buf[512];
                    if((n = read(socket_fd, buf, sizeof(buf))) < 0)
                    {
                        if(errno != EAGAIN)
                        {
                            error(1, errno, "read error");
                            close(socket_fd);
                        }
                        break;
                    }
                    else if(n == 0)
                    {
                        close(socket_fd);
                        break;
                    }
                    else
                    {
                        for(int i = 0; i < n; ++i)
                        {
                            buf[i] = rot13_char(buf[i]);
                        }
                        if(write(socket_fd, buf, n) < 0)
                        {
                            error(1, errno, "write error");
                        }
                    }
                }
               }
        }
    }

    delete[] events;
    close(listen_fd);
}

