cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(client tcpclient.cpp)
target_link_libraries(client yolanda)

add_executable(server tcpserver.cpp)
target_link_libraries(server yolanda)