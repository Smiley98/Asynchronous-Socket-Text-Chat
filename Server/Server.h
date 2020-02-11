#pragma once
#include "../Network/Network.h"
#include <vector>
#include <atomic>

enum State : unsigned char {
	//INIT,
	CONNECT,
	RUN,
	//QUIT
};

class Server : public Network
{
public:
	void init();
	void listen();
private:
	void run();
	void shutdown();
	void stateListener();

	std::vector<SOCKADDR_IN> m_clientAddresses;
	SOCKET m_socket = INVALID_SOCKET;
	std::atomic_uchar m_state;
};
