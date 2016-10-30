cmake_minimum_required(VERSION 3.0)
project(ev3-controller)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb")

option(CLIENT_PLATFORM "CLIENT_PLATFORM [DESKTOP,EV3]" DESKTOP)

add_definitions(-DASIO_STANDALONE=1)
add_definitions(-DASIO_HEADER_ONLY=1)
add_definitions(-DASIO_NO_TYPEID=1)

if(CLIENT_PLATFORM EQUAL "EV3")
  add_definitions(-DCLIENT_PLATFORM_EV3)
endif()

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

add_subdirectory(src/main)
