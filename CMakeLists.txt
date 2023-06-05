cmake_minimum_required(VERSION 3.9)
project(vmutils)
set(CMAKE_CXX_STANDARD 17)
add_library(VMUtils INTERFACE)
target_include_directories(VMUtils
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# build tests

option(VMUTILS_BUILD_TESTS "Set to ON to build tests" OFF)
if (VMUTILS_BUILD_TESTS)
  add_subdirectory(tests)
endif()

