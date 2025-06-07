add_executable(MultiThreadedServer
  MultiThreadedServer.cpp
)

target_link_libraries(MultiThreadedServer
  cppzmq
  spdlog::spdlog
)