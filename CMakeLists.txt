cmake_minimum_required(VERSION 3.9)
project(tcping)

set(CMAKE_CXX_STANDARD 11)

add_executable(tcping
        main.cpp
        tcp.cpp
        tcp.h)
