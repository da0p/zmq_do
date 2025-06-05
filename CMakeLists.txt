cmake_minimum_required(VERSION 3.22)

project(ZmqDo
    DESCRIPTION "ZMQ Application"
    LANGUAGES CXX C
)

include(CMake/setup.cmake)
include(CMake/findDependencies.cmake)
include(CMake/compilerFlags.cmake)
add_subdirectory(App)

include(CMake/sysinfo.cmake)