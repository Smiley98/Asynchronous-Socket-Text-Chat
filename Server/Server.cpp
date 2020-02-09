#include "Server.h"
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#define PACKET_LENGTH 512

bool compareAddresses(const SOCKADDR_IN& a, const SOCKADDR_IN& b) {
	return a.sin_addr.S_un.S_addr == b.sin_addr.S_un.S_addr && a.sin_port == b.sin_port;
}

void Server::init()
{
	WSADATA wsa;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (result != 0) {
		printf("Winsock2 initialization failed (%i).\n", result);
		shutdown();
	}

	addrinfo* addressInfo = NULL, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, "8888", &hints, &addressInfo) != 0) {
		printf("Address resolution failed (%i).\n", WSAGetLastError());
		shutdown();
	}

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET) {
		printf("Socket creation failed (%i).\n", WSAGetLastError());
		shutdown();
	}

	if (bind(m_socket, addressInfo->ai_addr, (int)addressInfo->ai_addrlen) == SOCKET_ERROR) {
		printf("Bind failed (%i).\n", WSAGetLastError());
		shutdown();
	}

	// 0 for blocking, 1 for non-blocking
	u_long mode = 1;
	if (ioctlsocket(m_socket, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("Socket I/O control failed (%i).\n", WSAGetLastError());
		shutdown();
	}
}

void Server::listen()
{
	printf("Listening. . .\n");
	while (true) {
		SOCKADDR_IN fromAddress;
		memset(&fromAddress, 0, sizeof(fromAddress));
		int fromAddressLength = sizeof(fromAddress);

		char packet[PACKET_LENGTH];
		memset(packet, 0, PACKET_LENGTH);

		if (recvfrom(m_socket, packet, PACKET_LENGTH, 0, (SOCKADDR*)&fromAddress, &fromAddressLength) != SOCKET_ERROR) {
			//\n is part of START because the input code appends \n.
			if (strcmp(packet, "START\n") == 0)
				run();
			printf("%s\n", packet);

			//Add the address if we have no clients, otherwise only add unique addresses (this sucks without maps).
			if (m_clientAddresses.empty()) {
				m_clientAddresses.push_back(fromAddress);
				char ipbuf[INET_ADDRSTRLEN];
				printf("%s connected.\n", inet_ntop(AF_INET, &fromAddress, ipbuf, sizeof(ipbuf)));
			}
			else {
				bool found = false;
				for (size_t i = 0; i < m_clientAddresses.size(); i++) {
					found |= compareAddresses(fromAddress, m_clientAddresses[i]);
				}
				if(!found)
				{
					m_clientAddresses.push_back(fromAddress);
					char ipbuf[INET_ADDRSTRLEN];
					printf("%s connected.\n", inet_ntop(AF_INET, &fromAddress, ipbuf, sizeof(ipbuf)));
				}
			}
		}
	}
}

//1. Determine which client a message came from.
//2. Send that message to any client but the sender.
//3. Profit.
void Server::run()
{	//while m_state == States::Transmitting
	printf("Transmitting. . .\n");
	while (true) {
		SOCKADDR_IN fromAddress;
		memset(&fromAddress, 0, sizeof(fromAddress));
		int fromAddressLength = sizeof(fromAddress);

		char packet[PACKET_LENGTH];
		memset(packet, 0, PACKET_LENGTH);

		//Test this tomorrow. Should simply send the text back to the opposite client!
		if (recvfrom(m_socket, packet, PACKET_LENGTH, 0, (SOCKADDR*)&fromAddress, &fromAddressLength) != SOCKET_ERROR) {
			if (strcmp(packet, "QUIT") == 0)
				shutdown();

			for (size_t i = 0; i < m_clientAddresses.size(); i++) {
				if (compareAddresses(fromAddress, m_clientAddresses[i]))
					continue;
				sendto(m_socket, packet, PACKET_LENGTH, 0, (SOCKADDR*)&m_clientAddresses[i], fromAddressLength);//Test if length is constant.
			}
		}
	}
}

void Server::shutdown()
{
	//freeaddrinfo(addressInfo);
	closesocket(m_socket);
	WSACleanup();
}
