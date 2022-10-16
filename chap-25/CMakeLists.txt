cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(fork01 fork01.cpp)
target_link_libraries(fork01 yolanda)