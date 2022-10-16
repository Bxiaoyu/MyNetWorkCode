#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

#define MAX_LINE 4096

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


void child_run(int fd)
{
    char outbuf[MAX_LINE + 1];
    size_t outbuf_used = 0;
    ssize_t result;

    while(1)
    {
        char ch;
        result = recv(fd, &ch, 1, 0);
        if(result == 0)
        {
            break;
        }
        else if(result == -1)
        {
            perror("read");
            break;
        }

        /* We do this test to keep the user from overflowing the buffer. */
        if(outbuf_used < sizeof(outbuf))
        {
            outbuf[outbuf_used++] = rot13_char(ch);
        }

        if(ch == '\n')
        {
            send(fd, outbuf, outbuf_used, 0);
            outbuf_used = 0;
            continue;
        }
    }
}

void sigchld_handler(int sig)
{
    // 值 -1 表示等待第一个终止的子进程
    while(waitpid(-1, 0, WNOHANG) > 0);
    return;
}

int main(int argc, char* argv[])
{
    int listenfd_fd;

    listenfd_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    int on = 1;
    setsockopt(listenfd_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int rt1 = bind(listenfd_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(rt1 < 0)
    {
        error(1, errno, "bind failed");
    }

    int rt2 = listen(listenfd_fd, LISTENQ);
    if(rt2 < 0)
    {
        error(1, errno, "listen failed");
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, sigchld_handler);

    while(1)
    {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof(ss);

        int fd = accept(listenfd_fd, (struct sockaddr*)&ss, &slen);
        if(fd < 0)
        {
            error(1, errno, "accept failed");
            exit(1);
        }

        if(fork() == 0)  // 进入子进程逻辑
        {
            close(listenfd_fd);  // 子进程不需要关心监听套接字，故而在这里关闭掉监听套接字 listen_fd
            child_run(fd);
            exit(0);
        }
        else  // 进入父进程逻辑
        {
            close(fd);  // 父进程不需要关心连接套接字，所以在这里关闭连接套接字
        }
    }

    return 0;
}