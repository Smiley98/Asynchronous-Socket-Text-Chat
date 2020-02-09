#pragma once
#include <winsock2.h>
#include <vector>

class Server
{
public:
	void init();
	void listen();
	void run();
	void shutdown();
private:
	std::vector<SOCKADDR_IN> m_clientAddresses;
	SOCKET m_socket = INVALID_SOCKET;
};
