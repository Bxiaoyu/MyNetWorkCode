#include <iostream>
#include <aio.h>

extern "C"
{
    #include "../lib/common.h"
}

const int BUF_SIZE = 512;

int main(int argc, char* argv[])
{
    int err;
    int result_size;

    // 创建一个临时文件
    char tmpname[256];
    snprintf(tmpname, sizeof(tmpname), "/tmp/aio_test_%d", getpid());
    unlink(tmpname);
    int fd = open(tmpname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if(fd == -1)
    {
        error(1, errno, "open file failed ");
    }
    unlink(tmpname);

    char buf[BUF_SIZE];
    aiocb aiob;

    // 初始化buf缓冲，写入数据应该为0xfafa这样的
    memset(buf, 0xfa, BUF_SIZE);
    memset(&aiob, 0, sizeof(aiocb));
    aiob.aio_fildes = fd;
    aiob.aio_buf = buf;
    aiob.aio_nbytes = BUF_SIZE;

    // 开始写
    if(aio_write(&aiob) == -1)
    {
        printf(" Error at aio_write(): %s\n", strerror(errno));
        close(fd);
        exit(1);
    }

    // 因为是异步，需要判断什么时候写完
    while(aio_error(&aiob) == EINPROGRESS)
    {
        printf("writing... \n");
    }

    // 判断写入的是否正确
    err = aio_error(&aiob);
    result_size = aio_return(&aiob);
    if(err != 0 || result_size != BUF_SIZE)
    {
        printf(" aio_write() failed: %s\n", strerror(err));
        close(fd);
        exit(1);
    }

    // 下面开始读数据
    char buffer[BUF_SIZE];
    aiocb cb;
    cb.aio_nbytes = BUF_SIZE;
    cb.aio_fildes = fd;
    cb.aio_offset = 0;
    cb.aio_buf = buffer;

    // 开始读数据
    if(aio_read(&cb) == -1)
    {
        printf(" aio_read() failed:%s \n", strerror(err));
        close(fd);
    }

    // 因为是异步，所以需要判断什么时候读完
    while(aio_error(&cb) == EINPROGRESS)
    {
        printf("Reading... \n");
    }

    // 判断是否读取成功
    int numBytes = aio_return(&cb);
    if(numBytes != -1)
    {
        printf("Success.\n");
    }
    else
    {
        printf("Error.\n");
    }

    // 清理文件句柄
    close(fd);
    return 0;
}