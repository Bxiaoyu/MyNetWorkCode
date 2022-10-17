#include <iostream>
#include "echo.h"

extern "C"
{
    #include "../lib/common.h"
}

void* thread_run(void *arg)
{
    /*
    在高并发的例子里，每个连接都由一个线程单独处理，在这种情况下，
    服务器程序并不需要对每个子线程进行终止，这样的话，每个子线程可以在入口函数开始的地方，
    把自己设置为分离的，这样就能在它终止后自动回收相关的线程资源了，就不需要调用 pthread_join 函数了。
    */
    pthread_detach(pthread_self());  // 分离线程
    int* fd = (int *)arg;
    loop_echo(*fd);
}

int main(int argc, char* argv[])
{
    int listen_fd = tcp_server_listen(SERV_PORT);
    pthread_t tid;

    while(1)
    {
        sockaddr_storage ss;
        socklen_t slen = sizeof(ss);
        int fd = accept(listen_fd, (struct sockaddr*)&ss, &slen);
        if(fd < 0)
        {
            error(1, errno, "accept failed");
        }
        else
        {
            pthread_create(&tid, NULL, thread_run, &fd);
        }
    }

    return 0;
}

