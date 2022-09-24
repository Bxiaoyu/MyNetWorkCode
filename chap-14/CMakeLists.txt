cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(client udpconnectclient.cpp)
target_link_libraries(client yolanda)

add_executable(client2 udpconnectclient2.cpp)
target_link_libraries(client2 yolanda)

add_executable(server udpconnectserver.cpp)
target_link_libraries(server yolanda)