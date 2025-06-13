add_executable(LazyPirateClient
  Client.cpp
)

target_include_directories(LazyPirateClient
  PUBLIC
    ./
)

target_link_libraries(LazyPirateClient
  cppzmq
  spdlog::spdlog
  ZmqUtil
)

add_executable(LazyPirateServer
  Server.cpp
)

target_include_directories(LazyPirateServer
  PUBLIC
    ./
)

target_link_libraries(LazyPirateServer
  cppzmq
  spdlog::spdlog
  ZmqUtil
  RandomNumberGenerator
)