#ifndef M_ACCEPTOR_H
#define M_ACCEPTOR_H
#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

// 封装连接类
class Acceptor
{
public:
    Acceptor(int port)
    {
        this->listen_port = port;
        this->listen_fd = socket(AF_INET, SOCK_STREAM, 0);

        // 设置非阻塞
        fcntl(this->listen_fd, F_SETFL, O_NONBLOCK);

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(port);

        // 设置地址重用
        int on = 1;
        setsockopt(this->listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        int rt1 = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(rt1 < 0)
        {
            error(1, errno, "bind failed ");
        }

        int rt2 = listen(listen_fd, LISTENQ);
        if(rt2 < 0)
        {
            error(1, errno, "listen failed ");
        }

        // signal(SIGPIPE, SIG_IGN);
    }

    /*获取连接描述字*/
    int get_fd() const
    {
        return this->listen_fd;
    }

    /*获取端口号*/
    int get_port() const
    {
        return this->listen_port;
    }
private:
    int listen_port;
    int listen_fd;
};
#endif