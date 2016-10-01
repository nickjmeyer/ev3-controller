add_library(networkWrapper networkWrapper.cpp)
target_link_libraries(networkWrapper
	${CMAKE_THREAD_LIBS_INIT})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS command.proto)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories($ENV{HOME}/libraries/asio/include)

# robot binaries
add_executable(ev3Server ev3Server.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(ev3Server networkWrapper
	${PROTOBUF_LIBRARY})

add_executable(ev3Client ev3Client.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(ev3Client networkWrapper
	${PROTOBUF_LIBRARY})
