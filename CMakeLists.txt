cmake_minimum_required(VERSION 3.0)
project(ev3-controller)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(-DASIO_STANDALONE=1)
add_definitions(-DASIO_HEADER_ONLY=1)
add_definitions(-DASIO_NO_TYPEID=1)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

add_subdirectory(src/main)
