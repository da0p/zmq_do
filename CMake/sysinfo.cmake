get_property(
  GLOBAL_COMPILE_OPTIONS
  DIRECTORY
  PROPERTY
  COMPILE_OPTIONS
)

message( STATUS "-------------------------------------------------------------")
message( STATUS "----- System Information -----")
message( STATUS "Host System Name:      ${CMAKE_HOST_SYSTEM_NAME}")
message( STATUS "Host System:           ${CMAKE_HOST_SYSTEM}")
message( STATUS "CMake Generator:       ${CMAKE_GENERATOR}")
message( STATUS "CMAKE_BUILD_TYPE:      ${CMAKE_BUILD_TYPE}")
message( STATUS "Compiler Info:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER}; version: ${CMAKE_CXX_COMPILER_VERSION}")
message( STATUS "CMAKE_SOURCE_DIR:      ${CMAKE_SOURCE_DIR}")
message( STATUS "CMAKE_BINARY_DIR:      ${CMAKE_BINARY_DIR}")
message( STATUS "CMAKE_MODULE_PATH:     ${CMAKE_MODULE_PATH}")
message( STATUS "CMAKE_PREFIX_PATH:     ${CMAKE_PREFIX_PATH}")
message( STATUS "")
message( STATUS "----- Compiler Flags -----")
message( STATUS "General:               ${CMAKE_CXX_FLAGS}")
message( STATUS "Debug:                 ${CMAKE_CXX_FLAGS_DEBUG}")
message( STATUS "Release:               ${CMAKE_CXX_FLAGS_RELEASE}")
message( STATUS "RelWithDebInfo:        ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message( STATUS "MinSizeRel:            ${CMAKE_CXX_FLAGS_MINSIZEREL}")
message( STATUS "Compile Options:       ${GLOBAL_COMPILE_OPTIONS}")
message( STATUS "")
message( STATUS "----- Global CMake Options -----")
message( STATUS "BUILD_SHARED_LIBS:     ${BUILD_SHARED_LIBS}")
message( STATUS "CMAKE_INSTALL_PREFIX:  ${CMAKE_INSTALL_PREFIX}")
message( STATUS "")