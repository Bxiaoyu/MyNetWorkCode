cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(server epoll_server_multithreads.cpp)
target_link_libraries(server yolanda pthread)