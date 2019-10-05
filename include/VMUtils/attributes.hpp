#pragma once

#include <utility>

#define VM_DEFINE_ATTRIBUTE( type, name )                     \
public:                                                       \
	template <typename... Args>                               \
	auto set_##name( Args &&... args )->decltype( ( *this ) ) \
	{                                                         \
		name = type( std::forward<Args>( args )... );         \
		return *this;                                         \
	}                                                         \
                                                              \
public:                                                       \
	type name
