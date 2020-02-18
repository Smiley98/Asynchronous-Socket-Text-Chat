#pragma once
#include "Server.h"
#include <atomic>

enum ServerState : byte {
	INITIALIZING = 0,
	CONNECTING,
	DISCONNECTING,
	RUNNING,
	QUIT
};

//All calls are blocking. Basically this class loops server calls till conditions for state transition are satisfied.
class ServerController
	: public Server
{
public:

private:
	std::atomic_uchar m_state = ServerState::INITIALIZING;
};
