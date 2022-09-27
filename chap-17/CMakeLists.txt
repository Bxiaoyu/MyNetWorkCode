cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(client01 reliable_client01.cpp)
target_link_libraries(client01 yolanda pthread)

add_executable(server01 reliable_server01.cpp)
target_link_libraries(server01 yolanda pthread)

add_executable(client02 reliable_client02.cpp)
target_link_libraries(client02 yolanda pthread)

add_executable(server02 reliable_server02.cpp)
target_link_libraries(server02 yolanda pthread)