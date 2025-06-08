add_executable(SyncPub
  SyncPub.cpp
)

target_link_libraries(SyncPub
  spdlog::spdlog
  cppzmq
)

add_executable(SyncSub
  SyncSub.cpp
)

target_link_libraries(SyncSub
  spdlog::spdlog
  cppzmq
)