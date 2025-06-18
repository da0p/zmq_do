add_library(RandomString
  RandomString.h
  RandomString.cpp
)

target_link_libraries(RandomString
  PRIVATE
    RandomNumberGenerator
)

target_include_directories(RandomString
  PUBLIC
    ./
)