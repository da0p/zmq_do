add_executable(Broker
  Broker.cpp
  BrokerState.cpp
)

target_include_directories(Broker
  PUBLIC
    ./
)

target_link_libraries(Broker
  cppzmq
  spdlog::spdlog
  ZmqUtil
  RandomNumberGenerator
)