#pragma once

#if defined( __GNUC__ )
#define VM_NO_INLINE __attribute__( ( noinline ) )
#elif defined( _MSC_VER )
#define VM_NO_INLINE __declspec( noinline )
#else
#define VM_NO_INLINE
#endif
