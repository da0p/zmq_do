add_library(ZmqUtil
  ZmqUtil.h
  ZmqUtil.cpp
)

target_link_libraries(ZmqUtil
  PRIVATE
    cppzmq
)

target_include_directories(ZmqUtil
  PUBLIC
    ./
)