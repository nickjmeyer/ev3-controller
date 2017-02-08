cmake_minimum_required(VERSION 3.0)
project(ev3-controller)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb")

option(BUILD_FOR_EV3 "BUILD_FOR_EV3" OFF)
option(EV3_SERVER_HOSTNAME "Hostname for server to control robots" OFF)
if(NOT EV3_SERVER_HOSTNAME)
  set(EV3_SERVER_HOSTNAME "localhost")
endif()

add_definitions(-DASIO_STANDALONE=1)
add_definitions(-DASIO_HEADER_ONLY=1)
add_definitions(-DASIO_NO_TYPEID=1)

include_directories(libs/asio/asio/include)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Glog REQUIRED)

if(BUILD_FOR_EV3)
  add_definitions(-DBUILD_FOR_EV3)
else()
  set(CURSES_NEED_NCURSES TRUE)
  find_package(Curses REQUIRED)
endif()

add_definitions(-DEV3_SERVER_HOSTNAME=${EV3_SERVER_HOSTNAME})

add_subdirectory(src/main)
