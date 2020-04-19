add_library(VMUtils INTERFACE)

target_include_directories(VMUtils
  INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(VMUtils INTERFACE cxx_std_14)

option(VMUTILS_INSTALL "install VMUtils headers" ON)
if (VMUTILS_INSTALL)
  install(
    DIRECTORY include/VMUtils
    DESTINATION include
  )
endif()
