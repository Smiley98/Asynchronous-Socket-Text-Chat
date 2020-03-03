#pragma once
#include "ServerBase.h"
#include <thread>
#include <atomic>

enum ServerState : byte {
	IDLE,
	ROUTE
};

class Server
	: public ServerBase
{
public:
	Server();
	~Server();

	void start();
	void stop();
	bool running();
	ServerState getState();
	void setState(ServerState serverState);

private:
	std::thread m_thread;
	std::atomic_uchar m_state = ServerState::IDLE;
	bool m_running = false;
	void run();
};
