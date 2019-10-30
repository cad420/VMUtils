#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <VMUtils/json_binding.hpp>

using namespace std;

struct A : vm::json::Serializable<A>
{
	VM_JSON_FIELD( int, a, 1 );
	VM_JSON_FIELD( string, b );
	VM_JSON_FIELD( vector<double>, c, {} );
};

TEST( test_json_binding, test_json_simple )
{
	{
		std::istringstream is( R"(
            {
                "a": 10,
                "b": "asd",
                "c": [1.0, 2.0, 3.0]
            }
        )" );
		A a;
		is >> a;
		ASSERT_EQ( a.a, 10 );
		ASSERT_EQ( a.b, "asd" );
		ASSERT_EQ( a.c, ( vector<double>{ 1., 2., 3. } ) );
	}
}

TEST( test_json_binding, test_json_type_mismatch )
{
	{
		std::istringstream is( R"(
            {
                "a": 10,
                "b": "asd",
                "c": "123"
            }
        )" );
		A a;
		ASSERT_THROW( is >> a, std::domain_error );
	}
	{
		std::istringstream is( R"(
            {
                "a": [1.0, 2.0, 3.0],
                "b": "12",
                "c": []
            }
        )" );
		A a;
		ASSERT_THROW( is >> a, std::domain_error );
	}
}

TEST( test_json_binding, test_json_default_value )
{
	{
		std::istringstream is( R"(
            {
                "b": "asd"
            }
        )" );
		A a;
		is >> a;
		ASSERT_EQ( a.a, 1 );
		ASSERT_EQ( a.b, "asd" );
		ASSERT_EQ( a.c, ( vector<double>{} ) );
	}
	{
		std::istringstream is( R"(
	        {
	        }
	    )" );
		A a;
		ASSERT_THROW( is >> a, std::domain_error );
	}
}

TEST( test_json_binding, test_json_nested )
{
	struct B : vm::json::Serializable<B>
	{
		VM_JSON_FIELD( A, a );
	};
	{
		std::istringstream is( R"(
            {
                "a" : {
                    "a": 10,
                    "b": "asd",
                    "c": [1]
                }
            }
        )" );
		B b;
		is >> b;
		ASSERT_EQ( b.a.a, 10 );
		ASSERT_EQ( b.a.b, "asd" );
		ASSERT_EQ( b.a.c, ( vector<double>{ 1 } ) );
	}
}

TEST( test_json_binding, test_json_as_array )
{
	struct B : vm::json::Serializable<B, vm::json::AsArray>
	{
		VM_JSON_FIELD( int, a, 1 );
		VM_JSON_FIELD( string, b );
		VM_JSON_FIELD( vector<string>, c, {} );
		using MapTy = map<string, int>;
		VM_JSON_FIELD( MapTy, d, {} );
	};
	{
		std::istringstream is( R"(
            [
                10,
                "asd",
                ["a", "b"],
                {
                    "a": 1,
                    "b": 2
                }
            ]
        )" );
		B b;
		is >> b;
		ASSERT_EQ( b.a, 10 );
		ASSERT_EQ( b.b, "asd" );
		ASSERT_EQ( b.c, ( vector<string>{ "a", "b" } ) );
		ASSERT_EQ( b.d, ( B::MapTy{ { "a", 1 }, { "b", 2 } } ) );
	}
	{
		std::istringstream is( R"(
            [
                10,
                "asd"
            ]
        )" );
		B b;
		is >> b;
		ASSERT_EQ( b.a, 10 );
		ASSERT_EQ( b.b, "asd" );
		ASSERT_EQ( b.c, ( vector<string>{} ) );
		ASSERT_EQ( b.d, ( B::MapTy{} ) );
	}
	{
		std::istringstream is( R"(
            [
            ]
        )" );
		B b;
		ASSERT_THROW( is >> b, std::domain_error );
	}
}