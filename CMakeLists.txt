cmake_minimum_required(VERSION 3.5)
project(Linux_CPU_Hotplugger)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(Linux_CPU_Hotplugger ${SOURCE_FILES})