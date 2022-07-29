cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "/home/anton/documents/YandexPracticum/protobuf")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")
file(GLOB PROTO_FILES "*.proto")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

source_group("Proto Files" ${PROTO_FILES})

add_executable(transport_catalogue
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${HEADERS}
    ${SOURCES}
    ${PROTO_FILES}
)

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
