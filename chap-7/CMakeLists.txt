cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(unixstreamclient unixstreamclient.cpp)
target_link_libraries(unixstreamclient yolanda)

add_executable(unixstreamserver unixstreamserver.cpp)
target_link_libraries(unixstreamserver yolanda)

add_executable(unixdataclient unixdataclient.cpp)
target_link_libraries(unixdataclient yolanda)

add_executable(unixdataserver unixdataserver.cpp)
target_link_libraries(unixdataserver yolanda)