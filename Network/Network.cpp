#include "Network.h"
//I'll need these if I lift connectivity functionality into here.
//#include <ws2tcpip.h>
//#include <memory>
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
