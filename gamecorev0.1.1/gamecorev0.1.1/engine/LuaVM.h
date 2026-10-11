#ifndef LUAVM_H
#define LUAVM_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct lua_State;

class LuaValue
{
public:
	LuaValue() : type_(Nil), number_(0.0) {}
	LuaValue(int number) : type_(Number), number_(number) {}
	LuaValue(double number) : type_(Number), number_(number) {}
	LuaValue(const char* text) : type_(String), number_(0.0), text_(text) {}
	LuaValue(const std::string& text) : type_(String), number_(0.0), text_(text) {}

	bool isNil() const { return type_ == Nil; }
	bool isNumber() const { return type_ == Number; }
	bool isString() const { return type_ == String; }
	double asNumber() const { return number_; }
	int asInt() const { return (int)number_; }
	const std::string& asString() const { return text_; }

private:
	enum Type { Nil, Number, String };
	Type type_;
	double number_;
	std::string text_;
};

class LuaVM
{
public:
	typedef std::function<void(const std::string&)> Logger;
	typedef std::function<std::vector<LuaValue>(const std::vector<LuaValue>&)> NativeFunction;

	LuaVM();
	~LuaVM();
	LuaVM(const LuaVM&) = delete;
	LuaVM& operator=(const LuaVM&) = delete;

	void setLogger(Logger logger) { logger_ = logger; }
	bool doString(const std::string& code);
	bool doFile(const std::string& path);
	bool call(const std::string& function, const std::vector<LuaValue>& args, std::vector<LuaValue>& results, int resultCount);
	void registerFunction(const std::string& name, NativeFunction function);

private:
	void logError(const std::string& context);

	lua_State* state_;
	Logger logger_;
	std::vector<std::unique_ptr<NativeFunction> > functions_;
};

#endif
