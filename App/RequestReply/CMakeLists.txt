add_executable(HelloServer
  server.cpp
)

target_link_libraries(HelloServer
  spdlog::spdlog
  cppzmq
)

add_executable(HelloClient
  client.cpp
)

target_link_libraries(HelloClient
  spdlog::spdlog
  cppzmq
)