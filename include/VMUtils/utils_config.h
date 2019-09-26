
#ifndef _UTILS_CONFIG_H_
#define _UTILS_CONFIG_H_


#if defined(_WIN32) && defined(VMUTILS_SHARED_LIBRARY)
#ifdef vmutils_EXPORTS
#define VMUTILS_EXPORTS __declspec(dllexport)
#else
#define VMUTILS_EXPORTS __declspec(dllimport)
#endif
#else
#define VMUTILS_EXPORTS
#endif

#endif