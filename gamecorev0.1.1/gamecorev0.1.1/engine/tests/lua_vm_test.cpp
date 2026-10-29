#include "doctest.h"
#include "LuaVM.h"
#include <string>
#include <vector>

namespace
{
	struct LoggedVM
	{
		LoggedVM() { vm.setLogger([this](const std::string& message) { log.push_back(message); }); }
		LuaVM vm;
		std::vector<std::string> log;
	};
}

TEST_CASE("doString runs code and reports success")
{
	LoggedVM lua;
	CHECK(lua.vm.doString("x = 1 + 1"));
	CHECK(lua.log.empty());
}

TEST_CASE("a syntax error fails and is logged")
{
	LoggedVM lua;
	CHECK_FALSE(lua.vm.doString("x = = 1"));
	REQUIRE(lua.log.size() == 1);
}

TEST_CASE("calling a Lua function returns its number result")
{
	LoggedVM lua;
	REQUIRE(lua.vm.doString("function double(n) return n * 2 end"));
	std::vector<LuaValue> results;
	CHECK(lua.vm.call("double", { LuaValue(21) }, results, 1));
	REQUIRE(results.size() == 1);
	CHECK(results[0].isNumber());
	CHECK(results[0].asInt() == 42);
}

TEST_CASE("int, float and string arguments arrive in order")
{
	LoggedVM lua;
	REQUIRE(lua.vm.doString("function describe(a, b, c) return a .. '|' .. b .. '|' .. c end"));
	std::vector<LuaValue> results;
	CHECK(lua.vm.call("describe", { LuaValue(3), LuaValue(1.5), LuaValue("clue") }, results, 1));
	REQUIRE(results.size() == 1);
	CHECK(results[0].asString() == "3|1.5|clue");
}

TEST_CASE("several results come back in order")
{
	LoggedVM lua;
	REQUIRE(lua.vm.doString("function three() return 1, 0, 90 end"));
	std::vector<LuaValue> results;
	CHECK(lua.vm.call("three", {}, results, 3));
	REQUIRE(results.size() == 3);
	CHECK(results[0].asNumber() == doctest::Approx(1.0));
	CHECK(results[1].asNumber() == doctest::Approx(0.0));
	CHECK(results[2].asNumber() == doctest::Approx(90.0));
}

TEST_CASE("a result the function did not return comes back as nil")
{
	LoggedVM lua;
	REQUIRE(lua.vm.doString("function one() return 7 end"));
	std::vector<LuaValue> results;
	CHECK(lua.vm.call("one", {}, results, 2));
	REQUIRE(results.size() == 2);
	CHECK(results[0].asInt() == 7);
	CHECK(results[1].isNil());
}

TEST_CASE("a string result is read as a string")
{
	LoggedVM lua;
	REQUIRE(lua.vm.doString("function message() return 'the killer is in the living room' end"));
	std::vector<LuaValue> results;
	CHECK(lua.vm.call("message", {}, results, 1));
	REQUIRE(results.size() == 1);
	CHECK(results[0].isString());
	CHECK(results[0].asString() == "the killer is in the living room");
}

TEST_CASE("calling a function that does not exist fails and is logged")
{
	LoggedVM lua;
	std::vector<LuaValue> results;
	CHECK_FALSE(lua.vm.call("missing", {}, results, 1));
	CHECK(results.empty());
	REQUIRE(lua.log.size() == 1);
	CHECK(lua.log[0].find("missing") != std::string::npos);
}

TEST_CASE("a runtime error fails, is logged, and the VM keeps working")
{
	LoggedVM lua;
	REQUIRE(lua.vm.doString("function broken() error('bo' .. 'om') end function fine() return 5 end"));
	std::vector<LuaValue> results;
	CHECK_FALSE(lua.vm.call("broken", {}, results, 1));
	REQUIRE(lua.log.size() == 1);
	CHECK(lua.log[0].find("boom") != std::string::npos);
	CHECK(lua.vm.call("fine", {}, results, 1));
	REQUIRE(results.size() == 1);
	CHECK(results[0].asInt() == 5);
}

TEST_CASE("globals persist between doString and later calls")
{
	LoggedVM lua;
	REQUIRE(lua.vm.doString("counter = 10"));
	REQUIRE(lua.vm.doString("function bump() counter = counter + 1 return counter end"));
	std::vector<LuaValue> results;
	CHECK(lua.vm.call("bump", {}, results, 1));
	CHECK(lua.vm.call("bump", {}, results, 1));
	REQUIRE(results.size() == 1);
	CHECK(results[0].asInt() == 12);
}

TEST_CASE("a registered C++ function can be called from Lua with arguments and returns results")
{
	LoggedVM lua;
	std::vector<LuaValue> seen;
	lua.vm.registerFunction("cpp_add", [&seen](const std::vector<LuaValue>& args) {
		seen = args;
		return std::vector<LuaValue>{ LuaValue(args[0].asNumber() + args[1].asNumber()), LuaValue("done") };
	});
	REQUIRE(lua.vm.doString("function useCpp() local sum, word = cpp_add(2, 3) return sum, word end"));
	std::vector<LuaValue> results;
	CHECK(lua.vm.call("useCpp", {}, results, 2));
	REQUIRE(seen.size() == 2);
	CHECK(seen[0].asInt() == 2);
	CHECK(seen[1].asInt() == 3);
	REQUIRE(results.size() == 2);
	CHECK(results[0].asInt() == 5);
	CHECK(results[1].asString() == "done");
}

TEST_CASE("doFile on a missing file fails and is logged")
{
	LoggedVM lua;
	CHECK_FALSE(lua.vm.doFile("scripts/does_not_exist.lua"));
	REQUIRE(lua.log.size() == 1);
	CHECK(lua.log[0].find("does_not_exist.lua") != std::string::npos);
}

TEST_CASE("errors without a logger do not crash")
{
	LuaVM vm;
	std::vector<LuaValue> results;
	CHECK_FALSE(vm.doString("x = = 1"));
	CHECK_FALSE(vm.call("missing", {}, results, 1));
}

TEST_CASE("the standard libraries are available to scripts")
{
	LoggedVM lua;
	REQUIRE(lua.vm.doString("function useLibs() return math.floor(2.7), string.upper('clue') end"));
	std::vector<LuaValue> results;
	CHECK(lua.vm.call("useLibs", {}, results, 2));
	REQUIRE(results.size() == 2);
	CHECK(results[0].asInt() == 2);
	CHECK(results[1].asString() == "CLUE");
}

TEST_CASE("scripts run on LuaJIT")
{
	LoggedVM lua;
	REQUIRE(lua.vm.doString("function runtime() return jit and jit.version or _VERSION end"));
	std::vector<LuaValue> results;
	CHECK(lua.vm.call("runtime", {}, results, 1));
	REQUIRE(results.size() == 1);
	CHECK(results[0].asString().find("LuaJIT 2.1.") == 0);
}
