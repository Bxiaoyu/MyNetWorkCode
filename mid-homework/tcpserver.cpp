#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>

extern "C"
{
    #include "../lib/common.h"
}

#define MAXNUMBER 16384

static int count;

static void sig_int(int signo)
{
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

// 列出文件
char* ls_cmd(char* path)
{
    char* res = new char[MAXNUMBER];
    memset(res, 0, MAXNUMBER);
    printf("path length: %d\n", strlen(path));
    DIR *dir = opendir(path);
    if(NULL == dir)
    {
        perror("opendir");
        printf("run %s error\n", path);
        return NULL;
    }

    char* tmp = res;
    struct dirent* name = NULL;
    while(name = readdir(dir))
    {
        strcpy(tmp, name->d_name);
        tmp += strlen(name->d_name);
        *tmp++ = '\n';
    }
    closedir(dir);  // 关闭文件句柄
    return res;
}

char* run_cmd(char* path)
{
    char* data = new char[MAXNUMBER];
    bzero(data, sizeof(data));
    FILE* fdp;
    const int max_buffer = 256;
    char buffer[max_buffer];
    fdp = popen(path, "r");
    char* data_index = data;
    if(fdp)
    {
        while(!feof(fdp))
        {
            if(fgets(buffer, max_buffer, fdp) != NULL)
            {
                int len = strlen(buffer);
                memcpy(data_index, buffer, len);
                data_index += len;
            }
        }
        pclose(fdp);
    }

    return data;
}

// 当前路径
char* pwd_cmd()
{
    char tmp[256];
    char* res = getcwd(tmp, 256);
    return res;
}

int main(int argc, char* argv[])
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    // memset(&server_addr, 0, sizeof(server_addr));
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int rt1 = bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(rt1 < 0)
    {
        error(1, errno, "bind failed ");
    }

    int rt2 = listen(listenfd, LISTENQ);
    if(rt2 < 0)
    {
        error(1, errno, "listen failed ");
    }

    signal(SIGPIPE, SIG_IGN);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    char buf[256];
    count = 0;

    while(1)
    {
        if((connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len)) < 0)
        {
            error(1, errno, "bind failed ");
        }

        while(1)
        {
            bzero(buf, sizeof(buf));
            int n = read(connfd, buf, sizeof(buf));
            if(n < 0)
            {
                error(1, errno, "error read message");
            }
            else if(n == 0)
            {
                printf("client closed \n");
                close(connfd);
                break;
            }
            count++;
            buf[n] = 0;
            if(strncmp(buf, "pwd", n) == 0)
            {
                char* res = pwd_cmd();
                if(send(connfd, res, strlen(res), 0) < 0)
                {
                    return 1;
                }
            }
            else if(strncmp(buf, "cd ", 3) == 0)
            {
                char target[256];
                bzero(target, sizeof(target));
                memcpy(target, buf+3, strlen(buf) - 3);
                if(chdir(target) == -1)
                {
                    printf("change dir failed, %s\n", target);
                }
            }
            else if(strncmp(buf, "ls", n) == 0)
            {
                char* res = ls_cmd("./");
                // char* res = run_cmd("ls");
                if(send(connfd, res, strlen(res), 0) < 0)
                {
                    return 1;
                }
                delete res;
                res = NULL;
            }
            else if(strncmp(buf, "ls ", 3) == 0)
            {
                char* res = ls_cmd(buf+3);
                if(send(connfd, res, strlen(res), 0) < 0)
                {
                    return 1;
                }
                delete res;
                res = NULL;
            }
            else
            {
                char* error = "error: unknown input type";
                if(send(connfd, error, strlen(error), 0) < 0)
                {
                    return 1;
                }
            }
        }
    }

    exit(0);
}

