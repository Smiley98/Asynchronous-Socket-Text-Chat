#pragma once
#include "Client.h"
#include <atomic>

enum ClientState : byte {
	IDLE,
	QUIT,
	CONNECT,
	DISCONNECT,
	CONSUME
};

class ClientController
{
public:
	void start();
	bool running();
	ClientState getState();
	void setState(ClientState clientState);

private:
	void run();
	Client m_client;
	std::atomic_uchar m_state = IDLE;
	bool m_running = false;
};
