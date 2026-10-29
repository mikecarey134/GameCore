#include "doctest.h"
#include "InputRouter.h"
#include <vector>

namespace
{
	struct RecordingLayer : InputLayer
	{
		RecordingLayer(std::vector<int>& log, int id, bool consumes) : log_(log), id_(id), consumes_(consumes), active_(true) {}
		bool onEvent(const irr::SEvent&) override { log_.push_back(id_); return consumes_; }
		bool isActive() const override { return active_; }
		std::vector<int>& log_;
		int id_;
		bool consumes_;
		bool active_;
	};

	irr::SEvent keyEvent(irr::EKEY_CODE key, bool pressed)
	{
		irr::SEvent event;
		event.EventType = irr::EET_KEY_INPUT_EVENT;
		event.KeyInput.Key = key;
		event.KeyInput.PressedDown = pressed;
		event.KeyInput.Char = 0;
		event.KeyInput.Shift = false;
		event.KeyInput.Control = false;
		return event;
	}
}

TEST_CASE("an event nobody consumes falls through so the GUI still gets it")
{
	std::vector<int> log;
	RecordingLayer layer(log, 1, false);
	InputRouter router;
	router.push(&layer);
	CHECK_FALSE(router.OnEvent(keyEvent(irr::KEY_KEY_W, true)));
	CHECK(log == std::vector<int>{ 1 });
}

TEST_CASE("the top layer sees an event first and can consume it")
{
	std::vector<int> log;
	RecordingLayer bottom(log, 1, false);
	RecordingLayer top(log, 2, true);
	InputRouter router;
	router.push(&bottom);
	router.push(&top);
	CHECK(router.OnEvent(keyEvent(irr::KEY_KEY_W, true)));
	CHECK(log == std::vector<int>{ 2 });
}

TEST_CASE("a consumed event never reaches lower layers")
{
	std::vector<int> log;
	RecordingLayer bottom(log, 1, false);
	RecordingLayer middle(log, 2, true);
	RecordingLayer top(log, 3, false);
	InputRouter router;
	router.push(&bottom);
	router.push(&middle);
	router.push(&top);
	CHECK(router.OnEvent(keyEvent(irr::KEY_KEY_W, true)));
	CHECK(log == std::vector<int>{ 3, 2 });
}

TEST_CASE("inactive layers are skipped")
{
	std::vector<int> log;
	RecordingLayer bottom(log, 1, false);
	RecordingLayer top(log, 2, true);
	top.active_ = false;
	InputRouter router;
	router.push(&bottom);
	router.push(&top);
	CHECK_FALSE(router.OnEvent(keyEvent(irr::KEY_KEY_W, true)));
	CHECK(log == std::vector<int>{ 1 });
}

TEST_CASE("the key state records presses and releases")
{
	InputRouter router;
	CHECK(router.state().isKeyUp(irr::KEY_KEY_W));
	router.OnEvent(keyEvent(irr::KEY_KEY_W, true));
	CHECK(router.state().isKeyDown(irr::KEY_KEY_W));
	CHECK(router.state().isKeyUp(irr::KEY_KEY_A));
	router.OnEvent(keyEvent(irr::KEY_KEY_W, false));
	CHECK(router.state().isKeyUp(irr::KEY_KEY_W));
}

TEST_CASE("the key state updates even when a layer consumes the event")
{
	std::vector<int> log;
	RecordingLayer top(log, 1, true);
	InputRouter router;
	router.push(&top);
	router.OnEvent(keyEvent(irr::KEY_KEY_W, true));
	CHECK(router.state().isKeyDown(irr::KEY_KEY_W));
}

TEST_CASE("a removed layer no longer receives events")
{
	std::vector<int> log;
	RecordingLayer bottom(log, 1, false);
	RecordingLayer top(log, 2, true);
	InputRouter router;
	router.push(&bottom);
	router.push(&top);
	router.remove(&top);
	CHECK_FALSE(router.OnEvent(keyEvent(irr::KEY_KEY_W, true)));
	CHECK(log == std::vector<int>{ 1 });
}
