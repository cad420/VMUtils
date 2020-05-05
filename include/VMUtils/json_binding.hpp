#pragma once

#include <type_traits>
#include <functional>
#include <iosfwd>
#include <string>
#include <memory>
#include <vector>
// #include <vec/vsel.hpp>
#include "nlohmann/json.hpp"
#include "modules.hpp"
#include "attributes.hpp"
#include "fmt.hpp"

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

template <typename T, typename = typename std::enable_if<
						std::is_base_of<
						  __flag::IsSerializable, T>::value>::type>
inline void to_json( nlohmann::json &j, const T &e )
{
	e.serialize( j );
}

template <typename T, typename = typename std::enable_if<
						std::is_base_of<
						  __flag::IsSerializable, T>::value>::type>
inline void from_json( const nlohmann::json &j, T &e )
{
	e.deserialize( j );
}

template <typename T>
inline void to_json( nlohmann::json &j, const std::shared_ptr<T> &e )
{
	if ( e ) {
		to_json( j, *e );
	}
}

template <typename T>
inline void from_json( const nlohmann::json &j, std::shared_ptr<T> &e )
{
	if ( !j.is_null() ) {
		e.reset( new T() );
		from_json( j, *e );
	} else {
		e = nullptr;
	}
}

#define VM_JSON_FIELD( T, name ) \
	VM_DEFINE_ATTRIBUTE( T, name ) = init_metadata<T>( #name, &type::name )

template <typename T, typename U>
struct AsObjectMetadata
{
	constexpr AsObjectMetadata( const char *name, U T::*offset ) :
	  name( name ),
	  offset( offset )
	{
	}

	operator U() const
	{
		if ( T::get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			T::get_components().emplace_back(
			  [=, name = this->name, offset = this->offset]( nlohmann::json &j, const T &t ) { j[ name ] = t.*offset; },
			  [=, name = this->name, offset = this->offset]( const nlohmann::json &j, T &t ) {
				  if ( j.count( name ) ) {
					  t.*offset = j.at( name ).template get<U>();
				  } else {
					  throw std::domain_error( vm::fmt( "No such key named {}", name ) );
				  }
			  } );
		}
		return U();
	}
	const U &operator=( const U &e ) const
	{
		if ( T::get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			T::get_components().emplace_back(
			  [=, name = this->name, offset = this->offset]( nlohmann::json &j, const T &t ) { j[ name ] = t.*offset; },
			  [=, name = this->name, offset = this->offset]( const nlohmann::json &j, T &t ) {
				  if ( j.count( name ) ) {
					  t.*offset = j.at( name ).template get<U>();
				  } else {
					  t.*offset = e;
				  }
			  } );
		}
		return e;
	}
	template <typename X, typename = typename std::enable_if<
							std::is_constructible<U, X>::value>::type>
	U operator=( X &&e ) const
	{
		U u( std::forward<X>( e ) );
		if ( T::get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			T::get_components().emplace_back(
			  [=, name = this->name, offset = this->offset]( nlohmann::json &j, const T &t ) { j[ name ] = t.*offset; },
			  [=, name = this->name, offset = this->offset]( const nlohmann::json &j, T &t ) {
				  if ( j.count( name ) ) {
					  t.*offset = j.at( name ).template get<U>();
				  } else {
					  t.*offset = u;
				  }
			  } );
		}
		return u;
	}

private:
	const char *const name;
	U T::*const offset;
};

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
	constexpr static AsObjectMetadata<T, U> init_metadata( const char *name, U T::*offset )
	{
		return AsObjectMetadata<T, U>( name, offset );
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
	template <typename X, typename Y>
	friend struct AsObjectMetadata;
};

template <typename T, typename U>
struct AsArrayMetadata
{
	constexpr AsArrayMetadata( const char *name, U T::*offset ) :
	  name( name ),
	  offset( offset )
	{
	}

	operator U() const
	{
		if ( T::get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			auto index = T::get_index();
			T::get_components().emplace_back(
			  [=, name = this->name, offset = this->offset]( nlohmann::json &j, const T &t ) { j[ index ] = t.*offset; },
			  [=, name = this->name, offset = this->offset]( const nlohmann::json &j, T &t ) {
				  if ( j.size() > index ) {
					  t.*offset = j.at( index ).template get<U>();
				  } else {
					  throw std::domain_error( vm::fmt( "No such key named {}", name ) );
				  }
			  } );
			T::get_index()++;
		}
		return U();
	}
	const U &operator=( const U &e ) const
	{
		if ( T::get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			auto index = T::get_index();
			T::get_components().emplace_back(
			  [=, name = this->name, offset = this->offset]( nlohmann::json &j, const T &t ) { j[ index ] = t.*offset; },
			  [=, name = this->name, offset = this->offset]( const nlohmann::json &j, T &t ) {
				  if ( j.size() > index ) {
					  t.*offset = j.at( index ).template get<U>();
				  } else {
					  t.*offset = e;
				  }
			  } );
			T::get_index()++;
		}
		return e;
	}
	template <typename X, typename = typename std::enable_if<
							std::is_constructible<U, X>::value>::type>
	U operator=( X &&e ) const
	{
		U u( std::forward<X>( e ) );
		if ( T::get_state() ==
			 1 )  // if this object is the first instance of this class
		{
			auto index = T::get_index();
			T::get_components().emplace_back(
			  [=, name = this->name, offset = this->offset]( nlohmann::json &j, const T &t ) { j[ index ] = t.*offset; },
			  [=, name = this->name, offset = this->offset]( const nlohmann::json &j, T &t ) {
				  if ( j.size() > index ) {
					  t.*offset = j.at( index ).template get<U>();
				  } else {
					  t.*offset = u;
				  }
			  } );
			T::get_index()++;
		}
		return u;
	}

private:
	const char *const name;
	U T::*const offset;
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
	constexpr static AsArrayMetadata<T, U> init_metadata( const char *name, U T::*offset )
	{
		return AsArrayMetadata<T, U>( name, offset );
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
	template <typename X, typename Y>
	friend struct AsArrayMetadata;
};

template <typename T, typename = typename std::enable_if<std::is_base_of<
						__flag::IsSerializable, T>::value>::type>
inline std::istream &operator>>( std::istream &is, T &t )
{
	nlohmann::json data;
	is >> data, t = data;
	return is;
}

struct Any : Serializable<Any>
{
	template <typename T>
	T get() const
	{
		return _.get<T>();
	}

	void update( nlohmann::json const &j )
	{
		_.update( j );
	}

	template <typename T>
	void update( T const &e )
	{
		nlohmann::json j;
		to_json( j, e );
		_.update( j );
	}

public:
	friend inline void to_json( nlohmann::json &j, Any const &e )
	{
		j = e._;
	}

	friend inline void from_json( nlohmann::json const &j, Any &e )
	{
		e._ = j;
	}

private:
	nlohmann::json _;
};

}  // namespace jsel

VM_EXPORT
{
	namespace json
	{
	using jsel::Any;
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
		string write( T const &t )
		{
			nlohmann::json j = t;
			if ( pretty ) {
				return j.dump( indent );
			} else {
				return j.dump( -1 );
			}
		}
		template <typename T>
		void write( std::ostream &os, T const &t )
		{
			os << write( t );
		}

		VM_JSON_FIELD( unsigned, indent ) = 4;
		VM_JSON_FIELD( bool, pretty ) = true;
	};

	}  // namespace json
}

VM_END_MODULE()
