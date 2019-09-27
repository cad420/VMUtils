#include <gtest/gtest.h>
#define NDEBUG
#include <log.hpp>

using namespace vm;
using namespace std;

TEST(test_log, test_log)
{
	Warning("warning {} {}", 1, "2");
	Log("log {} {}", 1, "2");
	Display("display {} {}", 1, "2");
	Debug("debug {} {}", 1, "2");
}
