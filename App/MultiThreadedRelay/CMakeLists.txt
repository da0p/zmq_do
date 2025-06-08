add_executable(MultiThreadedRelay
  MultiThreadedRelay.cpp
)

target_link_libraries(MultiThreadedRelay
  spdlog::spdlog
  cppzmq
)