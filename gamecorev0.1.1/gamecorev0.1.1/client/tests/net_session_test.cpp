#include "doctest.h"
#include "client/NetSession.h"
#include "net/EnetTransport.h"
#include "net/LoopbackTransport.h"
#include <chrono>
#include <thread>

using namespace protocol;

namespace
{
	const State still{ 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0 };

	template <typename ServerTransport, typename ClientTransport>
	struct Setup
	{
		template <typename... Args>
		explicit Setup(Args&... args)
			: serverTransport(args...), clientTransport(args...),
			  client(clientTransport, [this] { return now; }, NetClient::Events()),
			  session(serverTransport, client, [this] { return now; }, [](const std::string&) {}) {}

		bool joinedWithin(int rounds, bool sleep = false)
		{
			for (int i = 0; i < rounds && !client.isJoined(); ++i)
			{
				now += 10;
				session.tick();
				client.update(still);
				if (sleep)
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
			return client.isJoined();
		}

		uint32_t now = 1000;
		ServerTransport serverTransport;
		ClientTransport clientTransport;
		NetClient client;
		NetSession session;
	};

	struct Network
	{
		LoopbackNetwork network;
	};

	struct Loopback : Network, Setup<LoopbackTransport, LoopbackTransport>
	{
		Loopback() : Setup<LoopbackTransport, LoopbackTransport>(network) {}
	};
}

TEST_CASE("net session: host starts a server on the given port and joins it")
{
	Loopback setup;
	REQUIRE(setup.session.host(4100, "Dana", 1));
	CHECK(setup.session.isHosting());
	CHECK(setup.joinedWithin(20));

	LoopbackTransport other(setup.network);
	CHECK_FALSE(other.listen(4100, 4));
}

TEST_CASE("net session: host fails cleanly when the port is taken")
{
	Loopback setup;
	LoopbackTransport squatter(setup.network);
	REQUIRE(squatter.listen(4100, 4));
	CHECK_FALSE(setup.session.host(4100, "Dana", 1));
	CHECK_FALSE(setup.session.isHosting());
	CHECK_FALSE(setup.client.isOnline());
}

TEST_CASE("net session: join connects to a server at its address and port without hosting")
{
	Loopback setup;
	LoopbackTransport serverTransport(setup.network);
	uint32_t now = 1000;
	GameServer server(serverTransport, [&now] { return now; }, [](const std::string&) {});
	REQUIRE(server.start(4200));

	REQUIRE(setup.session.join("10.0.0.7", 4200, "Eli", 0));
	CHECK_FALSE(setup.session.isHosting());
	for (int i = 0; i < 20 && !setup.client.isJoined(); ++i)
	{
		server.tick();
		setup.client.update(still);
	}
	CHECK(setup.client.isJoined());
	CHECK(server.playerList() == std::vector<std::string>{ "1 Eli" });
}

TEST_CASE("net session: leave goes offline and frees the hosted port")
{
	Loopback setup;
	REQUIRE(setup.session.host(4100, "Dana", 1));
	REQUIRE(setup.joinedWithin(20));
	setup.session.leave();
	CHECK_FALSE(setup.session.isHosting());
	CHECK_FALSE(setup.client.isOnline());

	LoopbackTransport other(setup.network);
	CHECK(other.listen(4100, 4));
}

TEST_CASE("net session: hosting again after leaving works, and joining while hosting stops the server")
{
	Loopback setup;
	REQUIRE(setup.session.host(4100, "Dana", 1));
	setup.session.leave();
	REQUIRE(setup.session.host(4100, "Dana", 1));
	CHECK(setup.joinedWithin(20));

	LoopbackTransport serverTransport(setup.network);
	uint32_t now = 1000;
	GameServer server(serverTransport, [&now] { return now; }, [](const std::string&) {});
	REQUIRE(server.start(4200));
	REQUIRE(setup.session.join("10.0.0.7", 4200, "Dana", 1));
	CHECK_FALSE(setup.session.isHosting());
	LoopbackTransport other(setup.network);
	CHECK(other.listen(4100, 4));
}

TEST_CASE("net session: hosting while joined to another server leaves it first")
{
	Loopback setup;
	LoopbackTransport serverTransport(setup.network);
	uint32_t now = 1000;
	GameServer server(serverTransport, [&now] { return now; }, [](const std::string&) {});
	REQUIRE(server.start(4200));
	REQUIRE(setup.session.join("10.0.0.7", 4200, "Dana", 1));
	for (int i = 0; i < 20 && !setup.client.isJoined(); ++i)
	{
		server.tick();
		setup.client.update(still);
	}
	REQUIRE(server.playerList() == std::vector<std::string>{ "1 Dana" });

	REQUIRE(setup.session.host(4100, "Dana", 1));
	server.tick();
	CHECK(server.playerList().empty());
	CHECK(setup.session.isHosting());
	CHECK(setup.joinedWithin(20));
}

TEST_CASE("net session: hosting over real ENet joins through 127.0.0.1")
{
	Setup<EnetTransport, EnetTransport> setup;
	REQUIRE(setup.session.host(25617, "Dana", 1));
	CHECK(setup.joinedWithin(400, true));
	setup.session.leave();
}

TEST_CASE("net session: hosting takes a player limit and a clock for the server")
{
	LoopbackNetwork network;
	LoopbackTransport serverTransport(network), clientTransport(network);
	uint32_t now = 1000;
	float hours = -1.0f, scale = -1.0f;
	NetClient::Events events;
	events.clock = [&](float timeOfDay, float timeScale) { hours = timeOfDay; scale = timeScale; };
	NetClient client(clientTransport, [&now] { return now; }, events);
	NetSession session(serverTransport, client, [&now] { return now; }, [](const std::string&) {});
	session.setClock(18.0f, 72.0f);
	REQUIRE(session.host(4102, "Dana", 1, 1));
	for (int i = 0; i < 20 && !client.isJoined(); ++i)
	{
		now += 10;
		session.tick();
		client.update(still);
	}
	REQUIRE(client.isJoined());
	CHECK(hours == doctest::Approx(18.0f).epsilon(0.01));
	CHECK(scale == 72.0f);
	LoopbackTransport extra(network);
	CHECK_FALSE(extra.connect("localhost", 4102));
}
