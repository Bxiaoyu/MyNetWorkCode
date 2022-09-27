#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

#define MESSAGE_SIZE 102400000

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        error(1, 0, "usage: reliable_client <IPaddress>\n");
    }

    int socket_fd = tcp_client(argv[1], SERV_PORT);
    char buf[1024];
    int len;
    int rc;

    while (fgets(buf, 1024, stdin) != NULL)
    {
        len = strlen(buf);
        rc = send(socket_fd, buf, len, 0);
        if (rc < 0)
        {
            error(1, errno, "write failed!\n");
        }
        sleep(3);
        rc = read(socket_fd, buf, sizeof(buf));
        if(rc < 0)
        {
            error(1, errno, "read failed!\n");
        }
        else if(rc == 0)
        {
            error(1, 0, "peer connection closed\n");
        }
        else
        {
            fputs(buf, stdout);
        }
    }

    exit(0);
}