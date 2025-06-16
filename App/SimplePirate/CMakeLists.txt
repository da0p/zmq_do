add_executable(SimplePirateQueue
  SimplePirateQueue.cpp
)

target_link_libraries(SimplePirateQueue
  cppzmq
  spdlog::spdlog
  ZmqUtil
  RandomNumberGenerator
)

target_include_directories(SimplePirateQueue
  PRIVATE
    ./
)

add_executable(SimplePirateWorker
  SimplePirateWorker.cpp
)

target_link_libraries(SimplePirateWorker
  cppzmq
  spdlog::spdlog
  ZmqUtil
  RandomNumberGenerator
)

target_include_directories(SimplePirateWorker
  PRIVATE
    ./
)