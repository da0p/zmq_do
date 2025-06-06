add_executable(WeatherPublisher
  publisher.cpp
)

target_link_libraries(WeatherPublisher
  spdlog::spdlog
  cppzmq
)

add_executable(WeatherSubscriber
  subscriber.cpp
)

target_link_libraries(WeatherSubscriber
  spdlog::spdlog
  cppzmq
)