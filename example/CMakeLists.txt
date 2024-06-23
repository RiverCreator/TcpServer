get_filename_component(PARENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
include_directories(${PARENT_FOLDER}/public/include/RpcServer 
${PARENT_FOLDER}/public/include/TcpServer 
${PARENT_FOLDER}/public/include/Log
${PARENT_FOLDER}/public/include/Tool)
add_subdirectory(callee)
add_subdirectory(caller)
install(TARGETS consumer DESTINATION ${PARENT_FOLDER}/bin)
install(TARGETS provider DESTINATION ${PARENT_FOLDER}/bin)
INSTALL(FILES ${PARENT_FOLDER}/config/test.conf DESTINATION ${PARENT_FOLDER}/bin)