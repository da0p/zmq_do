add_executable(BinaryStarClient
  Client.cpp
  BinaryStarClient.cpp
)

target_link_libraries(BinaryStarClient
  PRIVATE
    cppzmq
    spdlog::spdlog
)

target_include_directories(BinaryStarClient
  PRIVATE
    ./
)

add_executable(BinaryStarServer
  Server.cpp
  BinaryStarServer.cpp
  BinaryStarFsm.cpp
)

target_link_libraries(BinaryStarServer
  PRIVATE
    cppzmq
    spdlog::spdlog
    magic_enum::magic_enum
)

target_include_directories(BinaryStarClient
  PRIVATE
    ./
)