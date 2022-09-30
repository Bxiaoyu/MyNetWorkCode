cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(server nonblockingserver.cpp)
target_link_libraries(server yolanda)