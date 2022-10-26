cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(main main.cpp buffer.h)
target_link_libraries(main yolanda)