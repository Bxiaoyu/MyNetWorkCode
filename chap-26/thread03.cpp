#include <iostream>
#include "echo.h"

extern "C"
{
    #include "../lib/common.h"
}

#define THREAD_NUMBER 4
#define BLOCK_QUEUE_SIZE 100

typedef struct {
    pthread_t thread_tid;  // thread ID
    long thread_count;     // connection handler
} Thread;

Thread *thread_array;

// 定义一个队列
typedef struct {
    int number;  // 队列里的描述字最大个数
    int* fd;     // 数组指针
    int front;   // 当前队列的头位置
    int rear;    // 当前队列的尾位置
    pthread_mutex_t mutex;  // 锁
    pthread_cond_t cond;    // 条件变量
} block_queue;


// 初始化队列
void block_queue_init(block_queue* blockQueue, int number)
{
    blockQueue->number = number;
    blockQueue->fd = new int[number];
    blockQueue->front = blockQueue->rear = 0;
    pthread_mutex_init(&blockQueue->mutex, NULL);
    pthread_cond_init(&blockQueue->cond, NULL);
}

// 销毁队列
void block_queue_destroy(block_queue* blockQueue)
{
    if(blockQueue->fd != NULL)
    {
        delete[] blockQueue->fd;
    }
    pthread_mutex_destroy(&blockQueue->mutex);
    pthread_cond_destroy(&blockQueue->cond);
}

// 往队列里放置一个描述字fd
void block_queue_push(block_queue* blockQueue, int fd)
{
    // 先加锁，因为有多个线程在读取
    pthread_mutex_lock(&blockQueue->mutex);
    // 将描述字放到队尾
    blockQueue->fd[blockQueue->rear] = fd;
    // 如果已经到队尾，重置队尾位置
    if(++blockQueue->rear == blockQueue->number)
    {
        blockQueue->rear = 0;
    }
    printf("push fd %d", fd);
    // 通知其它等待读的线程，有新的连接字等待处理
    pthread_cond_signal(&blockQueue->cond);
    // 解锁
    pthread_mutex_unlock(&blockQueue->mutex);
}

// 从队列里读出描述字进行处理
int block_queue_pop(block_queue* blockQueue)
{
    // 加锁
    pthread_mutex_lock(&blockQueue->mutex);
    // 判断队列里没有新的连接字可以处理，就一直条件等待，直到有新的连接字入队
    // 确保被pthread_cond_wait唤醒之后的线程，确实可以满足继续往下执行的条件。如果没有while循环的再次确认，可能直接就往下执行了
    while(blockQueue->front == blockQueue->rear)
    {
        pthread_cond_wait(&blockQueue->cond, &blockQueue->mutex);
    }
    // 取出队列头的连接字
    int fd = blockQueue->fd[blockQueue->front];
    // 如果已经到最后，重置头位置
    if(++blockQueue->front == blockQueue->number)
    {
        blockQueue->front = 0;
    }
    printf("pop fd %d", fd);
    // 解锁
    pthread_mutex_unlock(&blockQueue->mutex);
    // 返回连接字
    return fd;
}

void* thread_run(void* arg)
{
    pthread_t tid = pthread_self();
    pthread_detach(tid);

    block_queue* blockQueue = (block_queue*)arg;
    while(1)
    {
        int fd = block_queue_pop(blockQueue);
        printf("get fd in thread, fd==%d, tid==%d", fd, tid);
        loop_echo(fd);
    }
}

int main(int argc, char* argv[])
{
    int listen_fd = tcp_server_listen(SERV_PORT);

    block_queue blockQueue;
    block_queue_init(&blockQueue, BLOCK_QUEUE_SIZE);

    thread_array = new Thread[THREAD_NUMBER];
    for(int i = 0; i < THREAD_NUMBER; i++)
    {
        pthread_create(&(thread_array[i].thread_tid), NULL, thread_run, (void*)&blockQueue);
    }

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
            block_queue_push(&blockQueue, fd);
        }
    }

    block_queue_destroy(&blockQueue);
    delete[] thread_array;
    return 0;
}
