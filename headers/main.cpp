#include <iostream>
#include "buffer.h"

int main(int argc, char* argv[])
{
    Buffer buffer;
    char* s = "name";
    buffer.append(s, strlen(s));

    std::cout << "readable num = " << buffer.readable_size() << std::endl;
    std::cout << "writeable num = " << buffer.writeable_size() << std::endl;

    char* a = "hello";
    buffer.append(a, strlen(a));

    std::cout << "appended readable num = " << buffer.readable_size() << std::endl;
    std::cout << "appended writeable num = " << buffer.writeable_size() << std::endl;

    std::cout << "before front_spare_size = " << buffer.front_spare_size() << std::endl;
    int num = buffer.readable_size();
    for(int i = 0; i < num; i++)
    {
        if(i == num - 1)
        {
            std::cout << buffer.read_char() << std::endl;
        }
        else
        {
            std::cout << buffer.read_char() << " ";
        }
    }

    std::cout << "after front_spare_size = " << buffer.front_spare_size() << std::endl;

    std::cout << "readable num = " << buffer.readable_size() << std::endl;
    std::cout << "writeable num = " << buffer.writeable_size() << std::endl;

    return 0;
}