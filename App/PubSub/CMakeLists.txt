add_executable(WeatherPublisher
  publisher.cpp
)

target_link_libraries(WeatherPublisher
  spdlog::spdlog
  cppzmq
  RandomNumberGenerator
)

add_executable(WeatherSubscriber
  subscriber.cpp
)

target_link_libraries(WeatherSubscriber
  spdlog::spdlog
  cppzmq
)