cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(epollserver1 epoll_server1.cpp)
target_link_libraries(epollserver1 yolanda)

add_executable(epollserver2 epoll_server2.cpp)
target_link_libraries(epollserver2 yolanda)

add_executable(epollserver3 epoll_server3.cpp)
target_link_libraries(epollserver3 yolanda)