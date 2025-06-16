add_executable(ParanoidPirateQueue
  ParanoidPirateQueue.cpp
  WorkerQueue.cpp
)

target_include_directories(ParanoidPirateQueue
  PUBLIC
    ./
)

target_link_libraries(ParanoidPirateQueue
  cppzmq
  spdlog::spdlog
  ZmqUtil
  RandomNumberGenerator
)

add_executable(ParanoidPirateWorker
  ParanoidPirateWorker.cpp
)

target_include_directories(ParanoidPirateWorker
  PUBLIC
    ./
)

target_link_libraries(ParanoidPirateWorker
  cppzmq
  spdlog::spdlog
  ZmqUtil
  RandomNumberGenerator
)