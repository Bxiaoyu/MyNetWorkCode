cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(client streamclient.cpp)
target_link_libraries(client yolanda)

add_executable(server streamserver.cpp)
target_link_libraries(server yolanda)