add_library(networkWrapper networkWrapper.cpp)
target_link_libraries(networkWrapper
	${CMAKE_THREAD_LIBS_INIT})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS command.proto)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories($ENV{HOME}/libraries/asio/include)

if(BUILD_FOR_EV3 EQUAL "EV3")
  # client
  add_executable(ev3Client ev3Client.cpp ev3Robot.cpp ev3dev.cpp
    ${PROTO_SRCS} ${PROTO_HDRS})
  target_link_libraries(ev3Client networkWrapper
    ${PROTOBUF_LIBRARY})
else()
  # server
  add_executable(ev3Server ev3Server.cpp ${PROTO_SRCS} ${PROTO_HDRS})
  target_link_libraries(ev3Server networkWrapper
    ${PROTOBUF_LIBRARY} ${CURSES_LIBRARIES})

  # client
  add_executable(ev3Client ev3Client.cpp ev3Robot.cpp
    ${PROTO_SRCS} ${PROTO_HDRS})
  target_link_libraries(ev3Client networkWrapper
    ${PROTOBUF_LIBRARY})
endif()
