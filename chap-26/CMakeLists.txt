cmake_minimum_required(VERSION 3.10)

link_directories(/usr/lib/ ../lib/)

add_executable(thread_hello thread_hello.cpp)
target_link_libraries(thread_hello yolanda pthread)

add_executable(thread1 echo.h thread01.cpp)
target_link_libraries(thread1 yolanda pthread)

add_executable(thread3 echo.h thread03.cpp)
target_link_libraries(thread3 yolanda pthread)