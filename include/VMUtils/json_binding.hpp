#pragma once

#include <type_traits>
#include <functional>
#include <iosfwd>
#include <string>
#include <sstream>
#include <vector>
// #include <vec/vsel.hpp>
#include "nlohmann/json.hpp"
#include "modules.hpp"
#include "attributes.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;
using namespace nlohmann;

namespace jsel
{
struct AsObject
{
};

struct AsArray
{
};

namespace __flag
{
struct IsSerializable
{
};

}  // namespace __flag

template <typename T, typename = typename std::enable_if<std::is_base_of<
						__flag::IsSerializable, T>::value>::type>
inline void to_json( nlohmann::json &j, const T &e )
{
	e.serialize( j );
}

template <typename T, typename = typename std::enable_if<std::is_base_of<
						__flag::IsSerializable, T>::value>::type>
inline void from_json( const nlohmann::json &j, T &e )
{
	e.deserialize( j );
}

#define VM_JSON_FIELD( T, name, ... )                  \
	VM_DEFINE_ATTRIBUTE( T, name ) = init_metadata<T>( \
	  #name, &type::name, ##__VA_ARGS__ )  // if no init value the comma will be removed.

template <typename T, typename U = AsObject>
struct Serializable;

template <typename T>
struct Serializable<T, AsObject> : __flag::IsSerializable
{
	using type = T;

	Serializable()
	{
		switch ( get_state() ) {
		case 0:
			get_state() = 2, T(), get_state() = 3;
			break;
		case 2:
			get_state() = 1;
			break;
		}
	}
	void serialize( nlohmann::json &j ) const
	{
		j = nlohmann::json{};
		for ( auto &e : get_components() )
			e.first( j, static_cast<const T &>( *this ) );
	}
	void deserialize( const nlohmann::json &j )
	{
		for ( auto &e : get_components() ) e.second( j, static_cast<T &>( *this ) );
	}

protected:
	template <typename U>
	static U init_metadata( const std::string &name, U T::*offset,
							const U &default_value )
	{
		if ( get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			get_components().emplace_back(
			  [=]( nlohmann::json &j, const T &t ) { j[ name ] = t.*offset; },
			  [=]( const nlohmann::json &j, T &t ) {
				  if ( j.count( name ) ) {
					  t.*offset = j.at( name ).template get<U>();
				  } else {
					  t.*offset = default_value;
				  }
			  } );
		}
		return default_value;
	}
	template <typename U>
	static U init_metadata( const std::string &name, U T::*offset )
	{
		if ( get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			get_components().emplace_back(
			  [=]( nlohmann::json &j, const T &t ) { j[ name ] = t.*offset; },
			  [=]( const nlohmann::json &j, T &t ) {
				  if ( j.count( name ) ) {
					  t.*offset = j.at( name ).template get<U>();
				  } else {
					  throw std::domain_error( "No such key named \"" + name + "\"." );
				  }
			  } );
		}
		return U();
	}
	template <typename U, typename X>
	static U init_metadata( const std::string &name, U T::*offset,
							const X &default_value,
							const std::pair<std::function<U( const X & )>,
											std::function<X( const U & )>> &conv )
	{
		if ( get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			get_components().emplace_back(
			  [=]( nlohmann::json &j, const T &t ) {
				  j[ name ] = conv.second( t.*offset );
			  },
			  [=]( const nlohmann::json &j, T &t ) {
				  if ( j.count( name ) ) {
					  t.*offset = conv.first( j.at( name ).template get<X>() );
				  } else {
					  t.*offset = conv.first( default_value );
				  }
			  } );
		}
		return conv.first( default_value );
	}
	template <typename U, typename X>
	static U init_metadata( const std::string &name, U T::*offset,
							const std::pair<std::function<U( const X & )>,
											std::function<X( const U & )>> &conv )
	{
		if ( get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			get_components().emplace_back(
			  [=]( nlohmann::json &j, const T &t ) {
				  j[ name ] = conv.second( t.*offset );
			  },
			  [=]( const nlohmann::json &j, T &t ) {
				  if ( j.count( name ) ) {
					  t.*offset = conv.first( j.at( name ).template get<X>() );
				  } else {
					  throw std::domain_error( "No such key named \"" + name + "\"." );
				  }
			  } );
		}
		return U();
	}

private:
	static std::vector<
	  std::pair<std::function<void( nlohmann::json &, const T & )>,
				std::function<void( const nlohmann::json &, T & )>>>
	  &get_components()
	{
		static std::vector<
		  std::pair<std::function<void( nlohmann::json &, const T & )>,
					std::function<void( const nlohmann::json &, T & )>>>
		  val;
		return val;
	}
	static int &get_state()
	{
		static int state = 0;
		return state;
	}
};

template <typename T>
struct Serializable<T, AsArray> : __flag::IsSerializable
{
	using type = T;

	Serializable()
	{
		switch ( get_state() ) {
		case 0:
			get_state() = 2, T(), get_state() = 3;
			break;
		case 2:
			get_state() = 1;
			break;
		}
	}
	void serialize( nlohmann::json &j ) const
	{
		j = nlohmann::json{};
		for ( auto &e : get_components() )
			e.first( j, static_cast<const T &>( *this ) );
	}
	void deserialize( const nlohmann::json &j )
	{
		for ( auto &e : get_components() ) e.second( j, static_cast<T &>( *this ) );
	}

protected:
	template <typename U>
	static U init_metadata( const std::string &name, U T::*offset,
							const U &default_value )
	{
		if ( get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			auto index = get_index();
			get_components().emplace_back(
			  [=]( nlohmann::json &j, const T &t ) { j[ index ] = t.*offset; },
			  [=]( const nlohmann::json &j, T &t ) {
				  if ( j.size() > index ) {
					  t.*offset = j.at( index ).template get<U>();
				  } else {
					  t.*offset = default_value;
				  }
			  } );
			get_index()++;
		}
		return default_value;
	}
	template <typename U>
	static U init_metadata( const std::string &name, U T::*offset )
	{
		if ( get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			auto index = get_index();
			get_components().emplace_back(
			  [=]( nlohmann::json &j, const T &t ) { j[ index ] = t.*offset; },
			  [=]( const nlohmann::json &j, T &t ) {
				  if ( j.size() > index ) {
					  t.*offset = j.at( index ).template get<U>();
				  } else {
					  throw std::domain_error( "No such key named \"" + name + "\"." );
				  }
			  } );
			get_index()++;
		}
		return U();
	}

private:
	static std::vector<
	  std::pair<std::function<void( nlohmann::json &, const T & )>,
				std::function<void( const nlohmann::json &, T & )>>>
	  &get_components()
	{
		static std::vector<
		  std::pair<std::function<void( nlohmann::json &, const T & )>,
					std::function<void( const nlohmann::json &, T & )>>>
		  val;
		return val;
	}
	static int &get_state()
	{
		static int state = 0;
		return state;
	}
	static int &get_index()
	{
		static int index = 0;
		return index;
	}
};

template <typename T, typename = typename std::enable_if<std::is_base_of<
						__flag::IsSerializable, T>::value>::type>
inline std::istream &operator>>( std::istream &is, T &t )
{
	nlohmann::json data;
	is >> data, t = data;
	return is;
}

}  // namespace jsel

VM_EXPORT
{
	namespace json
	{
	using jsel::Serializable;

	using jsel::AsArray;
	using jsel::AsObject;

	template <typename T>
	inline T get( const nlohmann::json &json, const std::string &key, const T &val )
	{
		int start = 0, next;
		auto j = &json;
		do {
			next = key.find_first_of( '.', start );
			auto sub = key.substr( start, next );
			if ( j->count( sub ) ) {
				j = &j->at( sub );
			} else {
				return val;
			}
			start = next + 1;
		} while ( next != key.npos );
		return *j;
	}

	template <typename T>
	inline T get( const nlohmann::json &json, const std::string &key )
	{
		int start = 0, next;
		auto j = &json;
		do {
			next = key.find_first_of( '.', start );
			auto sub = key.substr( start, next );
			j = &j->at( sub );
			start = next + 1;
		} while ( next != key.npos );
		return *j;
	}

	struct Writer final : Serializable<Writer>
	{
		template <typename T>
		void write( std::ostream &os, T const &t )
		{
			nlohmann::json j = t;
			j.dump( os, pretty, indent, current_indent );
		}
		template <typename T>
		string write( T const &t )
		{
			std::ostringstream os;
			write( os, t );
			return os.str();
		}

		VM_JSON_FIELD( bool, pretty, false );
		VM_JSON_FIELD( unsigned, indent, 4 );
		VM_JSON_FIELD( unsigned, current_indent, 0 );
	};

	}  // namespace json
}

VM_END_MODULE()
