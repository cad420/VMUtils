#pragma once

#include <string>
#include "better_enums/enum.h"
#include "json_binding.hpp"

#define VM_ENUM( Enum, ... )                                  \
	BETTER_ENUM( Enum, int, __VA_ARGS__ )                     \
	inline void to_json( nlohmann::json &j, Enum const &e )   \
	{                                                         \
		j = std::string( e._to_string() );                    \
	}                                                         \
	inline void from_json( nlohmann::json const &j, Enum &e ) \
	{                                                         \
		auto str = j.get<std::string>();                      \
		e = Enum::_from_string( str.c_str() );                \
	}                                                         \
	struct __vm_enum_make_comma__impl
