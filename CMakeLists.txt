cmake_minimum_required(VERSION 3.9)
project(vmutils)

option(VMUTILS_BUILD_TESTS "Set to ON to build tests" OFF)

# install build scripts
find_package(Git)
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive)
# install external modules
include(scripts/external.cmake)
# export targets
include(export.cmake)

# build tests
if (VMUTILS_BUILD_TESTS)
  add_subdirectory(tests)
endif()

