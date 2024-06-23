cmake_minimum_required(VERSION 3.2)
project(mytcp)
get_filename_component(PARENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
get_filename_component(ROOT_FOLDER ${PARENT_FOLDER} DIRECTORY)
set(SOURCES 
    Acceptor.cpp 
    Channel.cpp 
    Connection.cpp 
    Epoll.cpp 
    EventLoop.cpp 
    InetAddress.cpp 
    Socket.cpp 
    TcpServer.cpp
    #${PARENT_FOLDER}/Tool/Buffer.cpp
    #${PARENT_FOLDER}/Tool/ThreadPool.cpp
    #${PARENT_FOLDER}/Tool/Timestamp.cpp
    #${PARENT_FOLDER}/Tool/HttpParse.cpp
)

set(INCLUDE_SOURCES
    ${ROOT_FOLDER}/public/include/TcpServer
    #${ROOT_FOLDER}/public/include/Log
    #${ROOT_FOLDER}/public/include/Tool
    )

include_directories(${INCLUDE_SOURCES})
add_library(mytcp SHARED ${SOURCES})
find_package(Threads REQUIRED)
target_link_libraries(mytcp mylog mytool pthread)
set_target_properties(mytcp PROPERTIES OUTPUT_NAME mytcp)

install(TARGETS mytcp DESTINATION ${ROOT_FOLDER}/public/lib)