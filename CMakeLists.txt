cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 14)
project(vmutils)

option(VMUTILS_BUILD_TESTS "Set to ON to build tests" OFF)

include(external.cmake)
include(export.cmake)

if (VMUTILS_BUILD_TESTS)
  find_package(GTest REQUIRED)
  file(GLOB_RECURSE VMUTILS_TEST_SOURCES
	"tests/*.cc"
	"tests/*.cpp"
  )
  add_executable(vmutils_test_all ${VMUTILS_TEST_SOURCES})
  vm_target_dependency(vmutils_test_all VMUtils PRIVATE)
  target_link_libraries(vmutils_test_all gtest_main gtest)
  include(GoogleTest)
  gtest_add_tests(vmutils_test_all "" AUTO)
endif()

