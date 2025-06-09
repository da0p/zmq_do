add_executable(Identity
  Identity.cpp
)

target_link_libraries(Identity
  spdlog::spdlog
  cppzmq
  ZmqUtil
)