//////////////////////////////////////////////////////////////////////////
/*
	InternalServer.cpp
	By : Michael Carey
	Purpose : provide communication to game Server from the client side
	in the forms of chat and player data
*/
//////////////////////////////////////////////////////////////////////////

#include "InternalServer.h"
#include <string>

using namespace irr;
using namespace video;

namespace
{
	std::string hostFrom(const char* ip)
	{
		std::string host = ip ? ip : "";
		size_t end = host.find_last_not_of(" \t\r\n");
		return end == std::string::npos ? "" : host.substr(0, end + 1);
	}

	SColor colourFor(NetClient::LineKind kind)
	{
		switch (kind)
		{
		case NetClient::JoinedLine:
		case NetClient::InfoLine:
			return SColor(255,0,255,0);
		case NetClient::LeftLine:
		case NetClient::DiedLine:
		case NetClient::ErrorLine:
			return SColor(255,255,0,0);
		default:
			return SColor(255,255,255,255);
		}
	}
}

InternalServer::InternalServer(bool online,irr::IrrlichtDevice* device,const char*,
							   irr::scene::ISceneManager* smgr,irr::video::IVideoDriver* driver,
							   gamecore::SoundEngine* engine, irrBulletWorld* world, char* ip,
							   int remote_port, int,ChatQueue* message_buffer,
							   player* thePlayer):
							   networkData_(device,smgr,driver,engine,world),
							   message_buffer_(message_buffer), player_(thePlayer),
							   started_(std::chrono::steady_clock::now()),
							   client_(transport_, [this] {
								   return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started_).count();
							   }, events())
{
	if(online)
		client_.connect(hostFrom(ip), (uint16_t)remote_port, thePlayer->getname(), (uint8_t)thePlayer->get_model_type());
}

InternalServer::~InternalServer(void)
{
}

float InternalServer::seconds() const
{
	return std::chrono::duration<float>(std::chrono::steady_clock::now() - started_).count();
}

NetClient::Events InternalServer::events()
{
	NetClient::Events events;
	events.line = [this](const std::string& text, NetClient::LineKind kind) {
		message_buffer_->addMessage(irr::core::stringw(text.c_str()), colourFor(kind));
	};
	events.joined = [this](const NetClient::Remote& remote) { networkData_.add(remote.id, remote.name, remote.model); };
	events.left = [this](uint16_t id) { networkData_.remove(id); };
	events.moved = [this](const NetClient::Remote& remote) { networkData_.place(remote.id, remote.state, seconds()); };
	events.died = [this](uint16_t id) { networkData_.hide(id); };
	events.respawned = [this](const NetClient::Remote& remote) { networkData_.show(remote.id); };
	events.damaged = [this](uint16_t) {
		if (!player_->isDead())
			player_->damage();
	};
	return events;
}

void InternalServer::messageLoop(player& thePlayer, irr::gui::IGUIFont*)
{
	if (client_.isOnline())
	{
		if(!emptyMess(thePlayer.getChatMessage()))
		{
			client_.sendChat(thePlayer.getChatMessage());
			thePlayer.setChatMessage("\0");
		}

		std::vector<uint16_t> hits = networkData_.takeDamaged();
		for (size_t i = 0; i < hits.size(); ++i)
			client_.sendHit(hits[i]);

		if(thePlayer.getCurrentState() == DEAD)
		{
			client_.sendDied();
			thePlayer.setState(SPECTATOR);
		}

		irr::core::vector3df position = thePlayer.getPosition();
		irr::core::vector3df rotation = thePlayer.getRotation();
		client_.update(protocol::State{ 0, { position.X, position.Y, position.Z }, { rotation.X, rotation.Y, rotation.Z },
			(uint8_t)thePlayer.get_curr_state() });
		networkData_.update(seconds());
	}
	else
	{
		if(!emptyMess(thePlayer.getChatMessage())) //if we have more than just the messageID aka its empty send
		{

			//display the current player's message in the buffer
			std::string buffered_message = "<";
			buffered_message+= thePlayer.getname();
			buffered_message+= "> ";
			buffered_message+= thePlayer.getChatMessage();

			thePlayer.setChatMessage("\0");
			message_buffer_->addMessage(buffered_message.c_str());
		}
	}
}

bool InternalServer::emptyMess(const char* message)
{
	int length=0;
	while(*message!= '\0')
	{
		++length;
		++message;
	}

	return length == 0;
}

void InternalServer::shutdown(player&)
{
	client_.shutdown();
	puts("Shutting Down Internal Server");
}
