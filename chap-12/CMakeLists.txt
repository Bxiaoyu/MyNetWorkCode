cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(pingclient pingclient.cpp)
target_link_libraries(pingclient yolanda)

add_executable(pingserver pingserver.cpp)
target_link_libraries(pingserver yolanda)