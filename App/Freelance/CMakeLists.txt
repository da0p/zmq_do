add_executable(FreelanceServer
  Server.cpp
  FreelanceServer.h
  FreelanceServer.cpp
)

target_link_libraries(FreelanceServer
  PRIVATE
    cppzmq
    spdlog::spdlog
    RandomString
    ZmqUtil
)

target_include_directories(FreelanceServer
  PRIVATE 
    ./
)

add_executable(FreelanceClient
  Client.cpp
  HeartbeatService.h
  HeartbeatService.cpp
  FreelanceClientProxy.h
  FreelanceClientProxy.cpp
  RequestService.h
  RequestService.cpp
)

target_link_libraries(FreelanceClient
  PRIVATE
    cppzmq
    spdlog::spdlog
    ZmqUtil
)

target_include_directories(FreelanceClient
  PRIVATE
    ./
)