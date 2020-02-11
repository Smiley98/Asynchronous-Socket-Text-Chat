#pragma once
#include <string>
#include "../Network/Network.h"

class Client : public Network
{
public:
	void init(const std::string& host = "localhost");
	void connect(bool& connected, bool& running);
	void send(const Packet& packet);
	//void start();
	PacketPool& packets();

private:
	void run();
	void shutdown();
	PacketPool m_packets;

	addrinfo* m_addressInfo = NULL;
	SOCKET m_socket = INVALID_SOCKET;
};
