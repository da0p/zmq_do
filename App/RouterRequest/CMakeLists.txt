add_executable(RouterRequest
  RouterRequest.cpp
)

target_link_libraries(RouterRequest
  spdlog::spdlog
  cppzmq
  ZmqUtil
  StopTimer
  RandomNumberGenerator
)