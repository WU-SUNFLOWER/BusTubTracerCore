set(SOCKET_SERVER_SOURCES socket_server.cpp)
add_executable(socket_server ${SOCKET_SERVER_SOURCES})

target_link_libraries(socket_server bustub)
set_target_properties(socket_server PROPERTIES OUTPUT_NAME socket_server)