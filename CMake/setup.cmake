# Set the C++ standard and require it
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create compile_commands.json to be used by clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the path for built binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Set build type to Debug")
  set(CMAKE_BUILD_TYPE "Debug")
endif()