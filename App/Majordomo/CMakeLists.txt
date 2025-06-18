add_executable(MajordomoClient
  Client.cpp
  MajordomoClient.cpp
)

target_link_libraries(MajordomoClient
  PRIVATE 
    cppzmq
    spdlog::spdlog
    ZmqUtil
    RandomString
)

target_include_directories(MajordomoClient
  PRIVATE
    ./
)

add_executable(MajordomoWorker
  Worker.cpp
  MajordomoWorker.cpp
)

target_link_libraries(MajordomoWorker
  PRIVATE
    cppzmq
    spdlog::spdlog
    ZmqUtil
    RandomString
)

target_include_directories(MajordomoWorker
  PRIVATE
    ./
)