cmake_minimum_required(VERSION 3.10)

add_executable(server poll_server_onethread.cpp)
target_link_libraries(server yolanda pthread)