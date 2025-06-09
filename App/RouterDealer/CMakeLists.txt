add_executable(RouterDealer
  RouterDealer.cpp
)

target_link_libraries(RouterDealer
  cppzmq
  spdlog::spdlog
  ZmqUtil
  RandomNumberGenerator
  StopTimer
)