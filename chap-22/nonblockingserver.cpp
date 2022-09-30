#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

#define MAX_LINE 1024
#define FD_INIT_SIZE 128

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

// 数据缓冲区
struct Buffer
{
    int connect_fd;         // 连接字
    char buffer[MAX_LINE];  // 实际缓冲
    size_t writeIndex;      // 缓冲写入位置
    size_t readIndex;       // 缓冲读取位置
    int readable;           // 是否可以读

    Buffer()
    {
        connect_fd = 0;
        writeIndex = readIndex = readable = 0;
    }
};

// 这里从fd套接字读取数据，数据先读到本地buf数组，再逐个拷贝到buffer对象缓冲区
int onSocketRead(int fd, Buffer* buffer)
{
    char buf[1024];
    ssize_t result;
    while(1)
    {
        result = recv(fd, buf, sizeof(buf), 0);
        if(result <= 0)
        {
            break;
        }

        // 按char对每个字节进行拷贝，每个字节都会调用rot13_char来完成编码，之后拷贝到buffer对象缓冲中
        // 其中writeIndex标志了缓冲中写的位置
        for(int i = 0; i < result; i++)
        {
            if(buffer->writeIndex < sizeof(buffer->buffer))
                buffer->buffer[buffer->writeIndex++] = rot13_char(buf[i]);
            // 如果读取了回车符，则认为client端发送结束，此时可以把编码后的数据回送给客户端
            if(buf[i] == '\n')
            {
                buffer->readable = 1;  // 缓冲区可读
            }
        }
    }

    if(result == 0)
    {
        return 1;
    }
    else if(result < 0)
    {
        if(errno == EAGAIN)
            return 0;
        return -1;
    }
    return 0;
}

// 从buffer对象的readIndex开始读，一直读到writeIndex的位置，这区间是有效数据
int onSocketWrite(int fd, Buffer* buffer)
{
    while(buffer->readIndex < buffer->writeIndex)
    {
        ssize_t result = send(fd, buffer->buffer + buffer->readIndex, buffer->writeIndex - buffer->readIndex, 0);
        if(result < 0)
        {
            if(errno == EAGAIN)
                return 0;
            return -1;
        }
        buffer->readIndex += result;
    }

    // readindex已经追上writeIndex，说明有效发送区间已经全部读完，将readIndex和writeIndex设置为0，复用这段缓冲
    if(buffer->readIndex == buffer->writeIndex)
        buffer->readIndex = buffer->writeIndex = 0;
    
    // 缓冲区数据已经全部读完，不需要再读
    buffer->readable = 0;

    return 0;
}

// 调用 fcntl 将监听套接字设置为非阻塞
void make_nonblock(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

// 初始化套接字
int init_socket(int port)
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // 调用 fcntl 将监听套接字设置为非阻塞
    make_nonblock(listenfd);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

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

    signal(SIGPIPE, SIG_IGN);

    return listenfd;
}

int main(int argc, char* argv[])
{
    int listen_fd;
    int j, maxfd;

    struct Buffer* buffer[FD_INIT_SIZE];
    for(int i = 0; i < FD_INIT_SIZE; i++)
    {
        buffer[i] = new Buffer;
    }

    listen_fd = init_socket(SERV_PORT);

    fd_set readset, writeset, exset;
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&exset);

    while(1)
    {
        maxfd = listen_fd;
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_ZERO(&exset);

        // listener加入readset
        FD_SET(listen_fd, &readset);

        for(int i = 1; i < FD_INIT_SIZE; i++)
        {
            if(buffer[i]->connect_fd > 0)
            {
                if(buffer[i]->connect_fd > maxfd)
                    maxfd = buffer[i]->connect_fd;
                FD_SET(buffer[i]->connect_fd, &readset);
                if(buffer[i]->readable)
                {
                    FD_SET(buffer[i]->connect_fd, &writeset);
                }
            }
        }

        if(select(maxfd + 1, &readset, &writeset, &exset, NULL) < 0)
        {
            error(1, errno, "select error");
        }

        if(FD_ISSET(listen_fd, &readset))
        {
            printf("listening socket readable\n");
            sleep(5);
            struct sockaddr_storage ss;
            socklen_t slen = sizeof(ss);
            int fd = accept(listen_fd, (struct sockaddr*)&ss, &slen);
            if(fd < 0)
            {
                error(1, errno, "accept failed");
            }
            else if(fd > FD_INIT_SIZE)
            {
                error(1, 0, "too many connections");
                close(fd);
            }
            else
            {
                make_nonblock(fd);
                if(buffer[fd]->connect_fd == 0)
                {
                    buffer[fd]->connect_fd = fd;
                }
                else
                {
                    error(1, 0, "too many connections");
                }
            }
        }

        for(int i = 0; i < maxfd + 1; i++)
        {
            int r = 0;
            if(i == listen_fd)
                continue;
            
            if(FD_ISSET(i, &readset))
            {
                r = onSocketRead(i, buffer[i]);
            }

            if(r == 0 && FD_ISSET(i, &writeset))
            {
                r = onSocketWrite(i, buffer[i]);
            }

            if(r)
            {
                buffer[i]->connect_fd = 0;
                close(i);
            }
        }
    }

    exit(0);
}