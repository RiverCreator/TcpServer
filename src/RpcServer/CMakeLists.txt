cmake_minimum_required(VERSION 3.2)
project(myrpc)
get_filename_component(PARENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
get_filename_component(ROOT_FOLDER ${PARENT_FOLDER} DIRECTORY)
set(SOURCES RpcApp.cpp  
    RpcChannel.cpp  
    RpcConfig.cpp  
    RpcController.cpp  
    RpcHeader.pb.cc 
    RpcProvider.cpp  
    zkClient.cpp
)

set(INCLUDE_SRC ${ROOT_FOLDER}/public/include/RpcServer
${ROOT_FOLDER}/public/include/TcpServer
#${ROOT_FOLDER}/public/include/Log
#${ROOT_FOLDER}/public/include/Tool
)
include_directories(${INCLUDE_SRC})
add_library(myrpc STATIC ${SOURCES})
find_package(Threads REQUIRED)
target_link_libraries(myrpc mylog mytcp pthread zookeeper_mt)
set_target_properties(myrpc PROPERTIES OUTPUT_NAME myrpc)

install(TARGETS myrpc DESTINATION ${ROOT_FOLDER}/public/lib)