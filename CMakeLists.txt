cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 14)
project(vmutils)

aux_source_directory(src SRC_LIST)

option(VMUTILS_SHARED_LIBRARY "Set to ON to build using dynamic linking" ON)
if(VMUTILS_SHARED_LIBRARY)
add_definitions(-DVMUTILS_SHARED_LIBRARY)
add_definitions(-Dvmutils_EXPORTS)
set(VMUTILS_SHARED "SHARED")
else()
set(VMUTILS_SHARED "STATIC")
endif()

add_library(vmutils ${VMUTILS_SHARED} ${SRC_LIST})
target_include_directories(vmutils PUBLIC "include")


install(TARGETS vmutils LIBRARY DESTINATION "lib" RUNTIME DESTINATION "bin" ARCHIVE DESTINATION "lib")
install(DIRECTORY "include" DESTINATION "include/VMutils")

