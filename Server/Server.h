#pragma once
#include "ServerBase.h"
#include <atomic>

enum ServerState : byte {
	IDLE,
	QUIT,
	CONNECT,
	DISCONNECT,
	ROUTE
};

class Server
	: public ServerBase
{
public:
	void start();
	bool running();
	ServerState getState();
	void setState(ServerState serverState);

private:
	void run();
	std::atomic_uchar m_state = ServerState::IDLE;
	bool m_running = false;
};
