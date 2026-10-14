#ifndef SCREENSTACK_H
#define SCREENSTACK_H

#include <vector>

template <typename Screen>
class ScreenStack
{
public:
	explicit ScreenStack(Screen root) : screens_(1, root) {}

	Screen top() const { return screens_.back(); }
	std::size_t depth() const { return screens_.size(); }

	void push(Screen screen) { screens_.push_back(screen); }

	bool back()
	{
		if (screens_.size() == 1)
			return false;
		screens_.pop_back();
		return true;
	}

	void reset(Screen root) { screens_.assign(1, root); }

private:
	std::vector<Screen> screens_;
};

#endif
