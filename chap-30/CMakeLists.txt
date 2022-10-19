cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(aio01 aio01.cpp)
target_link_libraries(aio01 yolanda)