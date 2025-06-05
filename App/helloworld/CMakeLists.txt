add_executable(HelloServer
  main.cpp
)

target_link_libraries(HelloServer
  spdlog::spdlog
  cppzmq
)