file(GLOB_RECURSE VMUTILS_TEST_SOURCES
  *.cc
  *.cpp
)
add_executable(vmutils_test_all ${VMUTILS_TEST_SOURCES})
vm_target_dependency(vmutils_test_all VMUtils PRIVATE)

find_package(GTest REQUIRED)
target_link_libraries(vmutils_test_all gtest_main gtest)
if (NOT CMAKE_CXX_COMPILER MATCHES MSVC)
  target_link_libraries(vmutils_test_all pthread)
endif() 
include(GoogleTest)
gtest_add_tests(vmutils_test_all "" AUTO)
