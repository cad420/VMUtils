cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 14)
project(vmutils)

option(VMUTILS_BUILD_TESTS "Set to ON to build tests" OFF)

install(DIRECTORY "include" DESTINATION "include/VMutils")

if (VMUTILS_BUILD_TESTS)
  find_package(GTest REQUIRED)
  file(GLOB_RECURSE VMUTILS_TEST_SOURCES
	"tests/*.cc"
	"tests/*.cpp"
  )
  add_executable(test_all ${VMUTILS_TEST_SOURCES})
  target_link_libraries(test_all gtest_main gtest)
  target_include_directories(test_all PRIVATE "include/VMUtils")
  include(GoogleTest)
  gtest_add_tests(test_all "" AUTO)
endif()
