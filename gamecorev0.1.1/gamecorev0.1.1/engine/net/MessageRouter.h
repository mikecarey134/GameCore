#ifndef MESSAGEROUTER_H
#define MESSAGEROUTER_H

#include <functional>
#include <string>
#include <vector>
#include "ByteStream.h"
#include "Transport.h"

class MessageRouter
{
public:
	typedef std::function<void(PeerId, ByteReader&)> Handler;
	typedef std::function<void(const std::string&)> Logger;

	explicit MessageRouter(Logger log);

	void on(uint8_t type, Handler handler);
	void route(PeerId peer, const std::vector<uint8_t>& packet);

private:
	Logger log_;
	Handler handlers_[256];
	bool loggedUnknown_[256];
	bool loggedMalformed_;
};

#endif
