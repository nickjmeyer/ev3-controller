add_library(networkWrapper networkWrapper.cpp)
target_link_libraries(networkWrapper
	${CMAKE_THREAD_LIBS_INIT})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS command.proto)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories($ENV{HOME}/libraries/asio/include)

# server
add_executable(ev3Server ev3Server.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(ev3Server networkWrapper
  ${PROTOBUF_LIBRARY} ${CURSES_LIBRARIES} ${GLOG_LIBRARIES})
