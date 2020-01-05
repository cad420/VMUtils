file(GLOB_RECURSE VMUTILS_TEST_SOURCES
  *.cc
  *.cpp
)
add_executable(vmutils_test_all ${VMUTILS_TEST_SOURCES})
vm_target_dependency(vmutils_test_all VMUtils PRIVATE)

enable_testing()
find_package(GTest CONFIG REQUIRED)
target_link_libraries(vmutils_test_all PRIVATE GTest::gtest_main GTest::gtest GTest::gmock GTest::gmock_main)
if (NOT CMAKE_CXX_COMPILER MATCHES MSVC)
  target_link_libraries(vmutils_test_all pthread)
endif()



include(GoogleTest)
gtest_add_tests(vmutils_test_all "" AUTO)


install(TARGETS vmutils_test_all LIBRARY DESTINATION "lib" RUNTIME DESTINATION "bin" ARCHIVE DESTINATION "lib")
