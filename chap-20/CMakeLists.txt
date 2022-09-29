cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(client select01.cpp)
target_link_libraries(client yolanda)