add_executable(ReqRepClient
  Client.cpp
)

target_link_libraries(ReqRepClient
  cppzmq
  spdlog::spdlog
  ZmqUtil
)

add_executable(ReqRepWorker
  Worker.cpp
)

target_link_libraries(ReqRepWorker
  cppzmq
  spdlog::spdlog
)

add_executable(ReqRepBroker
  Broker.cpp
)

target_link_libraries(ReqRepBroker
  cppzmq
  spdlog::spdlog
)

add_executable(MessageQueue
  MessageQueue.cpp
)

target_link_libraries(MessageQueue
  cppzmq
  spdlog::spdlog
)

add_executable(PubSubProxy
  PubSubProxy.cpp
)

target_link_libraries(PubSubProxy
  cppzmq
  spdlog::spdlog
)