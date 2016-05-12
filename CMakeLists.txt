cmake_minimum_required(VERSION 3.5)
project(Linux_CPU_Hotplugger)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(Linux_CPU_Hotplugger ${SOURCE_FILES})

find_package("PkgConfig")
pkg_check_modules(GTKMM "gtkmm-3.0")
include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})

set(SOURCE_FILES "GUI controller/MainApplication.cpp")
add_executable(GUI_Controller ${SOURCE_FILES})
target_link_libraries(GUI_Controller ${GTKMM_LIBRARIES})