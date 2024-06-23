cmake_minimum_required(VERSION 3.2)
project(httpserver)
get_filename_component(PARENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
get_filename_component(ROOT_FOLDER ${PARENT_FOLDER} DIRECTORY)
set(SOURCES hp_server.cpp httpserver.cpp)

set(INCLUDE_SOURCES
    ${ROOT_FOLDER}/public/include/TcpServer
    ${ROOT_FOLDER}/public/include/Log
    ${ROOT_FOLDER}/public/include/Tool)

include_directories(${INCLUDE_SOURCES})
find_package(Threads REQUIRED)
target_link_libraries(httpserver mytcp mylog pthread)
add_executable(httpserver ${SOURCES})
