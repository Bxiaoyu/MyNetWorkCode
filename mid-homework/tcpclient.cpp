#include <iostream>

extern "C"
{
    #include "../lib/common.h"
}

#define MAXLINE 1024

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        error(1, 0, "usgae: tcpclient <IPaddress> <Port>\n");
    }

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    // memset(&server_addr, 0, sizeof(server_addr));
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
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
    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);

    for(;;)
    {
        readmask = allreads;
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, NULL);

        if(rc <= 0)
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
                printf("server closed \n");
                break;
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

                if(strncmp(send_line, "quit", strlen(send_line)) == 0)
                {
                    if(shutdown(socket_fd, 1))
                    {
                        error(1, errno, "shutdown failed");
                    }
                }

                size_t rt = write(socket_fd, send_line, strlen(send_line));
                if(rt < 0)
                {
                    error(1, errno, "write failed ");
                }
            }
        }
    }

    exit(0);
}