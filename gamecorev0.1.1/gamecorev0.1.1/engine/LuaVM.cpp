#include "LuaVM.h"
#include <lua.hpp>
#include <iostream>

namespace
{
	LuaValue toValue(lua_State* state, int index)
	{
		int type = lua_type(state, index);
		if (type == LUA_TNUMBER)
			return LuaValue(lua_tonumber(state, index));
		if (type == LUA_TSTRING)
			return LuaValue(std::string(lua_tostring(state, index)));
		return LuaValue();
	}

	void pushValue(lua_State* state, const LuaValue& value)
	{
		if (value.isNumber())
			lua_pushnumber(state, value.asNumber());
		else if (value.isString())
			lua_pushstring(state, value.asString().c_str());
		else
			lua_pushnil(state);
	}

	int callNative(lua_State* state)
	{
		LuaVM::NativeFunction* function = (LuaVM::NativeFunction*)lua_touserdata(state, lua_upvalueindex(1));
		std::vector<LuaValue> args;
		for (int i = 1; i <= lua_gettop(state); ++i)
			args.push_back(toValue(state, i));
		std::vector<LuaValue> results = (*function)(args);
		for (size_t i = 0; i < results.size(); ++i)
			pushValue(state, results[i]);
		return (int)results.size();
	}
}

LuaVM::LuaVM() : state_(luaL_newstate())
{
	luaL_openlibs(state_);
}

LuaVM::~LuaVM()
{
	lua_close(state_);
}

bool LuaVM::doString(const std::string& code)
{
	if (luaL_dostring(state_, code.c_str()) != 0)
	{
		logError("Lua");
		return false;
	}
	return true;
}

bool LuaVM::doFile(const std::string& path)
{
	if (luaL_dofile(state_, path.c_str()) != 0)
	{
		logError(path);
		return false;
	}
	return true;
}

void LuaVM::registerFunction(const std::string& name, NativeFunction function)
{
	functions_.push_back(std::unique_ptr<NativeFunction>(new NativeFunction(function)));
	lua_pushlightuserdata(state_, functions_.back().get());
	lua_pushcclosure(state_, callNative, 1);
	lua_setglobal(state_, name.c_str());
}

bool LuaVM::call(const std::string& function, const std::vector<LuaValue>& args, std::vector<LuaValue>& results, int resultCount)
{
	results.clear();
	lua_getglobal(state_, function.c_str());
	for (size_t i = 0; i < args.size(); ++i)
		pushValue(state_, args[i]);
	if (lua_pcall(state_, (int)args.size(), resultCount, 0) != 0)
	{
		logError(function);
		return false;
	}
	for (int i = resultCount; i > 0; --i)
		results.push_back(toValue(state_, -i));
	lua_pop(state_, resultCount);
	return true;
}

void LuaVM::logError(const std::string& context)
{
	const char* message = lua_tostring(state_, -1);
	std::string text = context + ": " + (message ? message : "unknown error");
	lua_pop(state_, 1);
	if (logger_)
		logger_(text);
	else
		std::cerr << text << std::endl;
}
