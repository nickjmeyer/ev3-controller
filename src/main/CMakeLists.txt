add_library(networkWrapper networkWrapper.cpp)
target_link_libraries(networkWrapper
	${CMAKE_THREAD_LIBS_INIT})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS command.proto)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories($ENV{HOME}/libraries/asio/include)

# robot binaries
add_executable(robotServer robotServer.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(robotServer networkWrapper
	${PROTOBUF_LIBRARY})

add_executable(robotClient robotClient.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(robotClient networkWrapper
	${PROTOBUF_LIBRARY})
