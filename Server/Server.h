#pragma once
#include "ServerBase.h"
#include <atomic>

enum ServerState : byte {
	IDLE = 0,
	CONNECT,
	DISCONNECT,
	ROUTE,
	QUIT
};

class Server
	: public ServerBase
{
public:

private:
	std::atomic_uchar m_state = ServerState::IDLE;
};
