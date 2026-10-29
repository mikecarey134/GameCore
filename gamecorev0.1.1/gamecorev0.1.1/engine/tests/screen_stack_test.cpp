#include "doctest.h"
#include "ScreenStack.h"
#include <string>

namespace
{
	enum class Screen { Title, Options, Video, Lobby };
}

TEST_CASE("a new stack shows its root")
{
	ScreenStack<Screen> stack(Screen::Title);
	CHECK(stack.top() == Screen::Title);
	CHECK(stack.depth() == 1);
}

TEST_CASE("push shows the new screen and back returns through each one in turn")
{
	ScreenStack<Screen> stack(Screen::Title);
	stack.push(Screen::Options);
	stack.push(Screen::Video);
	CHECK(stack.top() == Screen::Video);
	CHECK(stack.depth() == 3);
	CHECK(stack.back());
	CHECK(stack.top() == Screen::Options);
	CHECK(stack.back());
	CHECK(stack.top() == Screen::Title);
}

TEST_CASE("back at the root does nothing and says so")
{
	ScreenStack<Screen> stack(Screen::Title);
	CHECK_FALSE(stack.back());
	CHECK(stack.top() == Screen::Title);
	CHECK(stack.depth() == 1);
}

TEST_CASE("reset leaves only the new root")
{
	ScreenStack<Screen> stack(Screen::Title);
	stack.push(Screen::Options);
	stack.push(Screen::Video);
	stack.reset(Screen::Lobby);
	CHECK(stack.top() == Screen::Lobby);
	CHECK(stack.depth() == 1);
	CHECK_FALSE(stack.back());
}

TEST_CASE("a game can name its screens with strings")
{
	ScreenStack<const char*> stack("title");
	stack.push("settings");
	CHECK(std::string(stack.top()) == "settings");
}
