#pragma once
#include "ClientBase.h"
#include <atomic>
#include <thread>

enum ClientState : byte {
	IDLE,
	CONSUME
};

class Client
	: public ClientBase
{
public:
	Client();
	~Client();

	void start();
	void stop();
	bool running();
	ClientState getState();
	void setState(ClientState clientState);

private:
	std::thread m_thread;
	std::atomic_uchar m_state = IDLE;
	bool m_running = false;
	void run();
};
