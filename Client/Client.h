#pragma once
#include "ClientBase.h"
#include <atomic>

enum ClientState : byte {
	IDLE,
	QUIT,
	CONNECT,
	DISCONNECT,
	CONSUME
};

class Client
	: public ClientBase
{
public:
	void start();
	bool running();
	ClientState getState();
	void setState(ClientState clientState);

private:
	void run();
	std::atomic_uchar m_state = IDLE;
	bool m_running = false;
};
