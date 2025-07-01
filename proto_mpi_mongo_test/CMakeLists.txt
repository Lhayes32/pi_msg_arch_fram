cmake_minimum_required(VERSION 3.16)
project(ProtoMongoMpiTest CXX)

# Find packages
find_package(MPI REQUIRED)
find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)
pkg_check_modules(BSONCXX REQUIRED libbsoncxx)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(SRC ${CMAKE_SOURCE_DIR}/src)
message(${SRC})

# Generate C++ from .proto
set(PROTO_FILES ${SRC}/proto/test_proto.proto)
add_custom_command(
    OUTPUT ${SRC}/test_proto.pb.cc ${SRC}/test_proto.pb.h
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${SRC} -I ${SRC} ${PROTO_FILES}
    COMMAND ${CMAKE_COMMAND} -E copy ${SRC}/proto/test_proto.pb.h ${SRC}/test_proto.pb.h
    COMMAND ${CMAKE_COMMAND} -E copy ${SRC}/proto/test_proto.pb.cc ${SRC}/test_proto.pb.cc
    DEPENDS ${PROTO_FILES}
    COMMENT "Generating C++ protobuf files in source dir"
)

set(CMAKE_BUILD_RPATH "/usr/local/lib")

# Executable
add_executable(test_api ${SRC}/main.cpp ${SRC}/test_proto.pb.cc ${SRC}/test_proto.pb.h)


# Includes
target_include_directories(test_api PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}       # For generated proto files
    ${Protobuf_INCLUDE_DIRS}
    ${MPI_CXX_INCLUDE_PATH}
    ${MONGOCXX_INCLUDE_DIRS}
    ${BSONCXX_INCLUDE_DIRS}
)

# Libraries
target_link_libraries(test_api
    ${Protobuf_LIBRARIES}
    ${MPI_CXX_LIBRARIES}
    ${MONGOCXX_LIBRARIES}
    ${BSONCXX_LIBRARIES}
)



# Install target binary to /bin, and any libraries to /lib
install(TARGETS test_api DESTINATION /bin)