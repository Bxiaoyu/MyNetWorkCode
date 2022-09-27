#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

int main(int argc, char* argv[])
{
    int connfd;
    char buf[1024];
    int time = 0;

    connfd = tcp_server(SERV_PORT);

    while(1)
    {
        int n = read(connfd, buf, 1024);
        if(n < 0)
        {
            error(1, errno, "error read");
        }
        else if(n == 0)
        {
            error(1, 0, "client closed \n");
        }

        time++;
        fprintf(stdout, "1k read for %d \n", time);
        usleep(1000);
    }

    exit(0);
}