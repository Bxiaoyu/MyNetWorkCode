#ifndef BUFFER_H
#define BUFFER_H
#include <iostream>
#include "../lib/common.h"

#define INIT_BUFFER_SIZE 65536

const char* CRLF = "\r\n";

// 数据缓冲区类
class Buffer
{
public:
    Buffer()
    {
        this->data = new char[INIT_BUFFER_SIZE];
        this->total_size = INIT_BUFFER_SIZE;
        this->readIndex = 0;
        this->writeIndex = 0;
    }

    ~Buffer()
    {
        if (this->data != nullptr)
            delete[] this->data;
    }

    // 还剩多少空间
    int writeable_size() const
    {
        return this->total_size - this->writeIndex;
    }

    // 剩余可读数据
    int readable_size() const
    {
        return this->writeIndex - this->readIndex;
    }

    // 已读数据的大小（即已处理的旧数据）
    int front_spare_size()
    {
        return this->readIndex;
    }

    // 往buffer写数据(任意类型数据)
    int append(void* data, int size)
    {}

    // 往buffer写数据(单个字符)
    int append_char(char data)
    {}

    // 往buffer里写数据(字符串)
    int append_string(char* data)
    {}

    // 读socket数据，往buffer写
    int socket_read(int fd)
    {}

    // 读buffer数据
    char read_char()
    {}

    // 查询buffer数据
    char* find_CRLF()
    {}

private:
    // 扩容
    void make_room(int size)
    {
        if(writeable_size() >= size)
            return;
        
        // 如果front_spare和writeable的大小加起来可以容纳数据，则把可读数据往前面拷贝
        if((front_spare_size() + writeable_size()) >= size)
        {
            int readable = readable_size();
            for(int i = 0; i < readable; ++i)
            {
                memcpy(this->data + i, this->data + this->readIndex + i, 1);
            }
            this->readIndex = 0;
            this->writeIndex = readable;
        }
    }

private:
    char* data;     // 实际缓冲
    int readIndex;  // 缓冲读取位置
    int writeIndex; // 缓冲写入位置
    int total_size; // 总大小
};

#endif