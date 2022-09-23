cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(batchwrite batchwrite.cpp)
target_link_libraries(batchwrite yolanda)