cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(udpclient udp_client.cpp)
target_link_libraries(udpclient yolanda)

add_executable(udpserver udp_server.cpp)
target_link_libraries(udpserver yolanda)