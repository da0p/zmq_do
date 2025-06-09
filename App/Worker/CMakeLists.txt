add_executable(TaskVentilator
  TaskVentilator.cpp
)

target_link_libraries(TaskVentilator
  spdlog::spdlog
  cppzmq
  RandomNumberGenerator
)

add_executable(TaskWorker
  TaskWorker.cpp
)

target_link_libraries(TaskWorker
  spdlog::spdlog
  cppzmq
)

add_executable(TaskSink
  TaskSink.cpp
)

target_link_libraries(TaskSink
  spdlog::spdlog
  cppzmq
  StopTimer
)