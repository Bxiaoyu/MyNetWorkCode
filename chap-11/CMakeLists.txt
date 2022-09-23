cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(graceclient graceclient.cpp)
target_link_libraries(graceclient yolanda)

add_executable(graceserver graceserver.cpp)
target_link_libraries(graceserver yolanda)