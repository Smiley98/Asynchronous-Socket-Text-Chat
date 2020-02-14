#include "Network.h"
//#include <ws2tcpip.h>//Needed if I lift connectivity into here.
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

void Network::startupWSA()
{
	WSADATA wsa;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (result != 0) {
		printf("Winsock2 initialization failed (%i).\n", result);
		cleanupWSA();
	}
}

void Network::cleanupWSA()
{
	WSACleanup();
}

Packet::Packet()
{
	init();
}

Packet::Packet(const Packet& packet)
{
	clone(packet);
}

Packet& Packet::operator=(const Packet& packet)
{
	clone(packet);
	return *this;
}
