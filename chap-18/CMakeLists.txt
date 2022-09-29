cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(s1 samplebuffer01.cpp)
target_link_libraries(s1 yolanda)

add_executable(s2 samplebuffer02.cpp)
target_link_libraries(s2 yolanda)

add_executable(s3 samplebuffer03.cpp)
target_link_libraries(s3 yolanda)