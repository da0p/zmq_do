add_executable(AsyncMajordomoClient
  Client.cpp
  MajordomoClient.cpp
)

target_link_libraries(AsyncMajordomoClient
  PRIVATE 
    cppzmq
    spdlog::spdlog
    ZmqUtil
    RandomString
    StopTimer
)

target_include_directories(AsyncMajordomoClient
  PRIVATE
    ./
)

add_executable(AsyncMajordomoWorker
  Worker.cpp
  MajordomoWorker.cpp
)

target_link_libraries(AsyncMajordomoWorker
  PRIVATE
    cppzmq
    spdlog::spdlog
    ZmqUtil
    RandomString
)

target_include_directories(AsyncMajordomoWorker
  PRIVATE
    ./
)

add_executable(AsyncMajordomoBroker
  Broker.cpp
  MajordomoBroker.cpp
)

target_link_libraries(AsyncMajordomoBroker
  PRIVATE
    cppzmq
    spdlog::spdlog
    ZmqUtil
    RandomString
)

target_include_directories(AsyncMajordomoBroker
  PRIVATE
    ./
)