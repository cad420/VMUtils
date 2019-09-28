#pragma once

#define VM_BEGIN_MODULE( mod )               \
	namespace mod                            \
	{                                        \
	namespace __inner__                      \
	{                                        \
	namespace __exported__                   \
	{                                        \
	}                                        \
	using namespace __exported__;            \
	} /*__inner__*/                          \
	using namespace __inner__::__exported__; \
	} /*mod*/                                \
	namespace mod                            \
	{                                        \
	namespace __inner__                      \
	{
#define VM_EXPORT \
	namespace __exported__

#define VM_END_MODULE() \
	} /*__inner__*/     \
	} /*mod*/
