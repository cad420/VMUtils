
#ifndef _COMMON_H_


#define ADD_EXPORT_DECL( moduleNameLowerCase, moduleNameUpperCase ) #if defined( _WIN32 ) && defined(#moduleNameUpperCase_SHARED_LIBRARY )\
#ifdef #moduleNameLowerCase_EXPORTS \
#define #moduleNameUpperCase_EXPORTS __declspec( dllexport )\
#else\
#define #moduleNameUpperCase_EXPORTS __declspec( dllimport )\
#endif           \                                                                                          
#else\                                                                                                       
#define #moduleNameUpperCase_EXPORTS \
#endif


#define _COMMON_H_
#endif