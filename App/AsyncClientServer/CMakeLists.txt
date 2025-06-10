add_executable(AsyncClientServer
  AsyncClientServer.cpp
)

target_link_libraries(AsyncClientServer
  cppzmq
  spdlog::spdlog
  ZmqUtil
  RandomNumberGenerator
)