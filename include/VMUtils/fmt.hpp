#pragma once

#include <string>
#include <cstdio>
#include <sstream>
#include <utility>
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <bitset>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <mutex>
#include <type_traits>
#include "bomb.hpp"
#include "modules.hpp"
#include "option.hpp"

VM_BEGIN_MODULE( vm )

using namespace std;

VM_EXPORT
{
	struct FmtErr : logic_error
	{
		FmtErr( string const &str ) :
		  logic_error( str ) {}
	};
	struct FmtSpec
	{
		static FmtSpec from_string( string const &spec )
		{
			auto _ = FmtSpec{};
			auto p = spec.data();

			enum class Stage : int
			{
				Start = 0,
				Align = 1,
				Sign = 2,
				Sharp = 3,
				Zero = 4,
				Width = 5,
				Prec = 6,
				Done = 7
			};
			Stage stage = Stage::Start;
			auto earlier_than = [&]( auto p, auto err ) {
				if ( !( static_cast<int>( stage ) < static_cast<int>( p ) ) ) {
					throw FmtErr( err + spec );
				}
			};

		PARSER:
			switch ( *p ) {
			case '<':
			case '^':
			case '>':
				earlier_than( Stage::Align, "unexpected align specification: " );
				p = get_align( p, _ );
				stage = Stage::Align;
				goto PARSER;
			case '+':
				earlier_than( Stage::Sign, "unexpected '+': " );
				_.sign = true;
				++p;
				stage = Stage::Sign;
				goto PARSER;
			case '#':
				earlier_than( Stage::Sharp, "unexpected '#': " );
				p = get_alter( p, _, spec );
				stage = Stage::Sharp;
				goto PARSER;
			case '0':
				earlier_than( Stage::Width, "unexpected '0': " );
				_.zero = true;
				++p;
				stage = Stage::Zero;
				goto PARSER;
			default:
				earlier_than( Stage::Width, "unexpected width specification: " );
				if ( static_cast<int>( stage ) < static_cast<int>( Stage::Align ) ) {
					// maybe this is an align
					switch ( p[ 1 ] ) {
					case '<':
					case '>':
					case '^':
						p = get_align( p, _ );
						stage = Stage::Align;
						goto PARSER;
					}
				}
				p = get_width( p, _, spec );
				stage = Stage::Width;
				goto PARSER;
			case '.':
				earlier_than( Stage::Prec, "unexpected '.': " );
				p = get_prec( p, _, spec );
				stage = Stage::Prec;
				goto PARSER;
			case 0: break;
			}
			return _;
		}

	private:
		static char const *get_number( char const *p, unsigned &n, string const &str )
		{
			if ( *p >= '1' && *p <= '9' ) {
				auto q = p;
				do {
					++p;
				} while ( *p >= '0' && *p <= '9' );
				char patt[ 10 ];
				sprintf( patt, "%%%uu", unsigned( p - q ) );
				sscanf( q, patt, &n );
			} else {
				throw FmtErr( "expected number: " + str );
			}
			return p;
		}
		static char const *get_prec( char const *p, FmtSpec &spec, string const &str )
		{
			unsigned n;
			p = get_number( p + 1, n, str );
			spec.precision = n;
			return p;
		}
		static char const *get_width( char const *p, FmtSpec &spec, string const &str )
		{
			unsigned n;
			p = get_number( p, n, str );
			spec.width = n;
			return p;
		}
		static char const *get_alter( char const *p, FmtSpec &spec, string const &str )
		{
			switch ( p[ 1 ] ) {
			case '?': spec.alter = FmtSpec::Alter::Debug; break;
			case 'X': spec.alter = FmtSpec::Alter::UpperHex; break;
			case 'x': spec.alter = FmtSpec::Alter::LowerHex; break;
			case 'b': spec.alter = FmtSpec::Alter::Bin; break;
			case 'o': spec.alter = FmtSpec::Alter::Oct; break;
			default: throw FmtErr( "expected one of [?Xxbo] after '#': " + str );
			}
			return p + 2;
		}
		static char const *get_align( char const *p, FmtSpec &spec )
		{
			spec.align = FmtSpec::Align{};
			switch ( p[ 1 ] ) {
			case '<':
			case '>':
			case '^':
				spec.align.value().fill = *p++;
			default: {
				switch ( *p++ ) {
				case '<': spec.align.value().type = FmtSpec::Align::Type::Left; break;
				case '>': spec.align.value().type = FmtSpec::Align::Type::Right; break;
				case '^': spec.align.value().type = FmtSpec::Align::Type::Center; break;
				}
			}
			}
			return p;
		}

	public:
		struct Align
		{
			enum class Type : char
			{
				Left = 0,
				Center = 1,
				Right = 2
			};
			Type type;
			Option<char> fill;
		};
		enum class Alter : char
		{
			Debug = 0,
			LowerHex = 1,
			UpperHex = 2,
			Bin = 3,
			Oct = 4
		};

	public:
		Option<Align> align;
		bool sign = false;
		Option<Alter> alter;
		bool zero = false;
		Option<size_t> width;
		Option<size_t> precision;
	};
}

template <typename T, bool IsInt, bool IsFloat>
struct FmtDefault;
template <typename T>
struct FmtDefault<T, false, false>
{
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		if ( spec.precision.has_value() ) {
			ostringstream ss;
			ss << t;
			auto prec = spec.precision.value();
			if ( ss.str().length() <= prec ) {
				os << ss.str();
			} else if ( spec.align.has_value() ) {
				switch ( spec.align.value().type ) {
				default:
				case FmtSpec::Align::Type::Left:
					os << ss.str().substr( 0, prec );
					break;
				case FmtSpec::Align::Type::Right:
					os << ss.str().substr( ss.str().length() - prec, prec );
					break;
				}
			} else {
				os << ss.str().substr( 0, prec );
			}
		} else {
			os << t;
		}
	}
};
template <typename T>
struct FmtDefault<T, true, false>  //int
{
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		auto old = os.flags();
		auto _ = make_bomb( [&] { os.flags( old ); } );
		os.flags( std::ios::dec );
		if ( spec.sign ) {
			os.setf( std::ios::showpos );
		}
		if ( spec.alter.has_value() ) {
			switch ( spec.alter.value() ) {
			case FmtSpec::Alter::Bin:  // bin is not implemented
				// os << "0b" << bitset<64>( t );
				os << bitset<64>( t );
				return;
			case FmtSpec::Alter::Oct:
				// os << "0o" << std::oct;
				os << std::oct;
				break;
			case FmtSpec::Alter::UpperHex:
				os << std::uppercase;
			case FmtSpec::Alter::LowerHex:
				// os << "0x" << std::hex;
				os << std::hex;
				break;
			}
		}
		os << t;
	}
};
template <typename T>
struct FmtDefault<T, false, true>  //fp
{
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		auto prec = os.precision();
		auto old = os.flags();
		auto _ = make_bomb( [&] { os.flags( old ); os.precision( prec ); } );
		if ( spec.sign ) {
			os.setf( std::ios::showpos );
		}
		if ( spec.precision.has_value() ) {
			os << std::fixed;
			os.precision( spec.precision.value() );
		}
		os << t;
	}
};

template <typename T>
struct FmtStrategy
{
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		using Default = FmtDefault<T, is_integral<T>::value, is_floating_point<T>::value>;
		ostringstream ss;
		Default::apply( ss, t, spec );
		auto old = os.flags();
		auto width = os.width();
		auto fill = os.fill();
		auto _ = make_bomb( [&] { os.flags( old ); os.width(width); os.fill(fill); } );
		if ( spec.align.has_value() ) {
			switch ( spec.align.value().type ) {
			case FmtSpec::Align::Type::Left: os << std::left; break;
			case FmtSpec::Align::Type::Right: os << std::right; break;
			default: break;
			}
			if ( spec.align.value().fill.has_value() ) {
				os.fill( spec.align.value().fill.value() );
			}
		}
		if ( spec.width.has_value() ) {
			os.width( spec.width.value() );
		}
		os << ss.str();
	}
};

template <typename T>
void fmt_impl( ostream &os, T &&t, FmtSpec const &spec = FmtSpec{} )
{
	using RmCV = typename remove_cv<T>::type;
	using RmRef = typename remove_reference<RmCV>::type;
	FmtStrategy<RmRef>::apply( os, std::forward<T>( t ), spec );
}

template <typename T>
void fmt_array( ostream &os, T const &t, size_t len, FmtSpec const &spec )
{
	os << "[";
	if ( len ) {
		fmt_impl( os, t[ 0 ] );
	}
	for ( auto i = 1; i < len; ++i ) {
		os << ", ";
		fmt_impl( os, t[ i ] );
	}
	os << "]";
}

template <typename T, typename A>
struct FmtStrategy<vector<T, A>>
{
	static void apply( ostream &os, vector<T, A> const &t, FmtSpec const &spec )
	{
		fmt_array( os, t, t.size(), spec );
	}
};
template <typename T, size_t N>
struct FmtStrategy<array<T, N>>
{
	static void apply( ostream &os, array<T, N> const &t, FmtSpec const &spec )
	{
		fmt_array( os, t, N, spec );
	}
};
template <typename T, size_t N>
struct FmtStrategy<T[ N ]>
{
	static void apply( ostream &os, T t[ N ], FmtSpec const &spec )
	{
		fmt_array( os, t, N, spec );
	}
};
template <size_t N>
struct FmtStrategy<char[ N ]> : FmtStrategy<char *>
{
};
template <size_t N>
struct FmtStrategy<const char[ N ]> : FmtStrategy<const char *>
{
};
template <size_t N>
struct FmtStrategy<volatile char[ N ]> : FmtStrategy<volatile char *>
{
};
template <size_t N>
struct FmtStrategy<const volatile char[ N ]> : FmtStrategy<const volatile char *>
{
};

template <typename T>
void fmt_map( ostream &os, T const &t, FmtSpec const &spec )
{
	os << "{";
	bool first = true;
	for ( auto &e : t ) {
		auto &k = e.first;
		auto &v = e.second;
		if ( !first ) {
			os << ", ";
		}
		fmt_impl( os, k );
		os << ": ";
		fmt_impl( os, v );
		first = false;
	}
	os << "}";
}

template <typename K, typename V, typename C, typename A>
struct FmtStrategy<map<K, V, C, A>>
{
	static void apply( ostream &os, map<K, V, C, A> const &t, FmtSpec const &spec )
	{
		fmt_map( os, t, spec );
	}
};
template <typename K, typename V, typename H, typename E, typename A>
struct FmtStrategy<unordered_map<K, V, H, E, A>>
{
	static void apply( ostream &os, unordered_map<K, V, H, E, A> const &t, FmtSpec const &spec )
	{
		fmt_map( os, t, spec );
	}
};

template <typename T>
void fmt_set( ostream &os, T const &t, FmtSpec const &spec )
{
	os << "{";
	bool first = true;
	for ( auto &e : t ) {
		if ( !first ) {
			os << ", ";
		}
		fmt_impl( os, e );
		first = false;
	}
	os << "}";
}

template <typename K, typename C, typename A>
struct FmtStrategy<set<K, C, A>>
{
	static void apply( ostream &os, set<K, C, A> const &t, FmtSpec const &spec )
	{
		fmt_set( os, t, spec );
	}
};
template <typename K, typename H, typename E, typename A>
struct FmtStrategy<unordered_set<K, H, E, A>>
{
	static void apply( ostream &os, unordered_set<K, H, E, A> const &t, FmtSpec const &spec )
	{
		fmt_set( os, t, spec );
	}
};

template <size_t K, size_t N>
struct FmtAtK
{
	template <typename T>
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		os << ", ";
		fmt_impl( os, std::get<K>( t ), spec );
		FmtAtK<K + 1, N>::apply( os, t, spec );
	}
};
template <size_t N>
struct FmtAtK<0, N>
{
	template <typename T>
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		os << "(";
		fmt_impl( os, std::get<0>( t ), spec );
		FmtAtK<1, N>::apply( os, t, spec );
	}
};
template <size_t N>
struct FmtAtK<N, N>
{
	template <typename T>
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		os << ")";
	}
};
template <>
struct FmtAtK<0, 0>
{
	template <typename T>
	static void apply( ostream &os, T const &t, FmtSpec const &spec )
	{
		os << "()";
	}
};
template <typename A, typename B>
struct FmtStrategy<pair<A, B>>
{
	static void apply( ostream &os, pair<A, B> const &t, FmtSpec const &spec )
	{
		FmtAtK<0, 2>::apply( os, t, spec );
	}
};
template <typename... Args>
struct FmtStrategy<tuple<Args...>>
{
	static void apply( ostream &os, tuple<Args...> const &t, FmtSpec const &spec )
	{
		FmtAtK<0, sizeof...( Args )>::apply( os, t, spec );
	}
};

struct FmtImpl
{
	template <typename T, typename... Rest>
	static void apply( ostream &os, string const &raw, const char *patt,
					   T &&t, Rest &&... rest )
	{
		patt = move_fn(
		  patt, raw,
		  [&]( auto _ ) { fmt_impl( os, std::forward<T>( t ), FmtSpec::from_string( _ ) ); },
		  [&]( auto _ ) { os << _; } );
		if ( patt ) {
			apply( os, raw, patt, std::forward<Rest>( rest )... );
		}
	}
	static void apply( ostream &os, string const &raw, const char *patt )
	{
		while ( patt = move_fn(
				  patt, raw, [&]( auto _ ) {}, [&]( auto _ ) { os << _; } ) )
			;
	}
	template <typename F, typename G>
	static const char *move_fn( const char *patt, string const &raw, F &&f, G &&g )
	{
		for ( auto p = patt; *p; ++p ) {
			switch ( *p ) {
			case '{': {
				if ( p[ 1 ] != '{' ) {
					ostringstream os;
					for ( auto q = p + 1; *q; ++q ) {
						if ( *q == '}' ) {
							if ( q[ 1 ] != '}' ) {
								f( os.str() );
								return q + 1;
							}
							++q;
						}
						os << *q;
					}
					// should be error {
					throw std::logic_error( "invalid format template: \"" + raw + "\"" );
				}
				++p;
			} break;
			case '}': {
				if ( p[ 1 ] != '}' ) {
					throw std::logic_error( "invalid format template: \"" + raw + "\"" );
				}
				++p;
			} break;
			}
			g( *p );
		}
		return nullptr;
	}
};

VM_EXPORT
{
	// fmt
	template <typename... Args>
	string fmt( string const &patt, Args &&... args )
	{
		ostringstream os;
		FmtImpl::apply( os, patt, patt.data(), std::forward<Args>( args )... );
		return os.str();
	}
}

struct FmtProxy
{
	template <typename... Args>
	string operator()( Args &&... args ) const
	{
		return fmt( str, std::forward<Args>( args )... );
	}
	constexpr FmtProxy( const char *str, size_t N ) :
	  str( str ),
	  N( N ) {}

private:
	const char *str;
	size_t N;
};

inline std::recursive_mutex &cout_lock()
{
	static std::recursive_mutex _;
	return _;
}

inline std::recursive_mutex &cerr_lock()
{
	static std::recursive_mutex _;
	return _;
}

VM_EXPORT
{
	// print
	template <typename... Args>
	void fprint( ostream & os, string const &patt, Args &&... args )
	{
		FmtImpl::apply( os, patt, patt.data(), std::forward<Args>( args )... );
	}
	template <typename... Args>
	void print( string const &patt, Args &&... args )
	{
		std::unique_lock<std::recursive_mutex> _( cout_lock() );
		fprint( cout, patt, std::forward<Args>( args )... );
	}
	template <typename... Args>
	void eprint( string const &patt, Args &&... args )
	{
		std::unique_lock<std::recursive_mutex> _( cerr_lock() );
		fprint( cerr, patt, std::forward<Args>( args )... );
	}
	// println
	template <typename... Args>
	void fprintln( ostream & os, string const &patt, Args &&... args )
	{
		fprint( os, patt, std::forward<Args>( args )... );
		os << endl;
	}
	template <typename... Args>
	void println( string const &patt, Args &&... args )
	{
		std::unique_lock<std::recursive_mutex> _( cout_lock() );
		fprintln( cout, patt, std::forward<Args>( args )... );
	}
	template <typename... Args>
	void eprintln( string const &patt, Args &&... args )
	{
		std::unique_lock<std::recursive_mutex> _( cerr_lock() );
		fprintln( cerr, patt, std::forward<Args>( args )... );
	}

	constexpr FmtProxy operator""_fmt( const char str[], size_t N )
	{
		return FmtProxy( str, N );
	}
}

VM_END_MODULE()
