cmake_minimum_required(VERSION 4.0)

project(ZmqDo
    DESCRIPTION "ZMQ Application"
    LANGUAGES CXX C
)

include(CMake/setup.cmake)
include(CMake/findDependencies.cmake)
include(CMake/compilerFlags.cmake)
add_subdirectory(App)
add_subdirectory(Shared)

include(CMake/sysinfo.cmake)