#pragma once

#if ( __cplusplus > 201703L )  // c++20
#define VM_CXX_GE_20
#endif

#if ( __cplusplus >= 201703L )  // c++17
#define VM_CXX_GE_17
#endif

#if ( __cplusplus >= 201402L )  // c++14
#define VM_CXX_GE_14
#endif

#if ( __cplusplus >= 201103L )  // c++11
#define VM_CXX_GE_11
#endif
