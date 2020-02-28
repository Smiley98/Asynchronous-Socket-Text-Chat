#include "Network.h"
#pragma comment(lib, "Ws2_32.lib")
#include <cstdio>

void Network::initialize()
{
	WSADATA wsa;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (result != 0) {
		printf("Winsock2 initialization failed (%i).\n", result);
		shutdown();
	}
}

void Network::shutdown()
{
	WSACleanup();
}

SOCKET Network::createSocket(bool blocking)
{
	SOCKET soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (soc == INVALID_SOCKET) {
		printf("Socket creation failed (%i).\n", WSAGetLastError());
		shutdown();
	}

	u_long mode = blocking ? 0 : 1;
	if (ioctlsocket(soc, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("Socket I/O control failed (%i).\n", WSAGetLastError());
		destroySocket(soc);
		shutdown();
	}

	return soc;
}

void Network::destroySocket(SOCKET soc)
{
	if (closesocket(soc) == INVALID_SOCKET) {
		printf("Socket close failed (%i).\n", WSAGetLastError());
		shutdown();
	}
}

ADDRINFO* const Network::createAddress(bool bind, const std::string& host, const std::string& port)
{
	ADDRINFO* addressInfo = NULL, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = bind ? AI_PASSIVE : 0;
	const char* nodeName = host.empty() ? NULL : host.c_str();
	const char* serviceName = port.empty() ? NULL : port.c_str();
	if (getaddrinfo(nodeName, serviceName, &hints, &addressInfo) != 0) {
		printf("Address resolution failed (%i).\n", WSAGetLastError());
		shutdown();
	}
	return addressInfo;
}

void Network::destroyAddress(ADDRINFO* const address)
{
	freeaddrinfo(address);
}

void Network::bindSocket(SOCKET soc, ADDRINFO* const address)
{
	if (bind(soc, address->ai_addr, static_cast<int>(address->ai_addrlen)) == SOCKET_ERROR) {
		printf("Bind failed (%i).\n", WSAGetLastError());
		destroyAddress(address);
		destroySocket(soc);
		shutdown();
	}
}

std::vector<size_t> findPacketOfType(PacketType packetType, const PacketBuffer& packetBuffer)
{
	std::vector<size_t> indices;
	for (size_t i = 0; i < packetBuffer.size(); i++) {
		if (packetBuffer[i].getType() == packetType)
			indices.push_back(i);
	}
	return indices;
}
