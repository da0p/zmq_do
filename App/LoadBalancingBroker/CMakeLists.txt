add_executable(LoadBalancingBroker
  LoadBalancingBroker.cpp
)

target_link_libraries(LoadBalancingBroker
  cppzmq
  spdlog::spdlog
  ZmqUtil
)