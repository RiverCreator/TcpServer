cmake_minimum_required(VERSION 3.2)
# project(server)
set(CMAKE_CXX_STANDARD 14)
add_subdirectory(src)
#add_subdirectory(example)

# file(GLOB_RECURSE SOURCES 
#     "${PROJECT_SOURCE_DIR}/src/Log/*.cpp" 
#     "${PROJECT_SOURCE_DIR}/src/TcpServer/*.cpp"
#     "${PROJECT_SOURCE_DIR}/src/RpcServer/*.cpp"
#     "${PROJECT_SOURCE_DIR}/src/Tool/*.cpp"
#     "${PROJECT_SOURCE_DIR}/src/HttpServer/*.cpp"
# )

# set(LOG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/public/include/Log)
# set(TcpServer_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/public/include/TcpServer)
# set(RpcServer_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/public/include/RpcServer)
# set(Tool_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/public/include/Tool)
# set(HttpServer_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/public/include/HttpServer)

# set(BIN_DIR ${PROJECT_SOURCE_DIR}/public/bin)
# add_executable(${PROJECT_NAME} ${SOURCES})
# target_include_directories(${PROJECT_NAME} PRIVATE 
#         ${LOG_INCLUDE_DIR}
#         ${TcpServer_INCLUDE_DIR}
#         ${RpcServer_INCLUDE_DIR}
#         ${Tool_INCLUDE_DIR}
#         ${HttpServer_INCLUDE_DIR}

# ) #设置包含目录
# find_package(Threads REQUIRED)
# target_link_libraries(${PROJECT_NAME} pthread)
# add_custom_target(CopyRuntimeFiles ALL
#         VERBATIM
#         COMMAND_EXPAND_LISTS
#                 COMMAND ${CMAKE_COMMAND} -E
#                         make_directory ${PROJECT_BINARY_DIR}/deploy/
#                 COMMAND ${CMAKE_COMMAND} -E
#                         make_directory ${PROJECT_BINARY_DIR}/install/)
# #INSTALL(FILES ${CONFIG_FILES} DESTINATION ${PROJECT_BINARY_DIR}/install/config)
# INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/install)