add_executable(InterBroker
  main.cpp
  BrokerState.cpp
  Client.cpp
  Worker.cpp
  WorkRouter.cpp
)

target_include_directories(InterBroker
  PUBLIC
    ./
)

target_link_libraries(InterBroker
  cppzmq
  spdlog::spdlog
  ZmqUtil
  RandomNumberGenerator
)