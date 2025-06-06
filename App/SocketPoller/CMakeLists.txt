add_executable(SocketPoller
  SocketPoller.cpp
)

target_link_libraries(SocketPoller
  spdlog::spdlog
  cppzmq
)