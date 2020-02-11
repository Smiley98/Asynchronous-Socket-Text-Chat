#include "Server.h"
#include <ws2tcpip.h>
#include <thread>
#pragma comment(lib, "Ws2_32.lib")

bool compareAddresses(const SOCKADDR_IN& a, const SOCKADDR_IN& b) {
	return a.sin_addr.S_un.S_addr == b.sin_addr.S_un.S_addr && a.sin_port == b.sin_port;
}

void Server::init()
{
	startupWSA();

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

	u_long mode = 1;
	if (ioctlsocket(m_socket, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("Socket I/O control failed (%i).\n", WSAGetLastError());
		shutdown();
	}

	std::thread(&Server::stateListener, this).detach();
}

void Server::listen()
{
	printf("Listening. . .\n");
	m_state.store(State::CONNECT);
	while (m_state.load() == State::CONNECT) {
		SOCKADDR_IN fromAddress;
		memset(&fromAddress, 0, sizeof(fromAddress));
		int fromAddressLength = sizeof(fromAddress);

		char packet[PACKET_LENGTH];
		memset(packet, 0, PACKET_LENGTH);

		if (recvfrom(m_socket, packet, PACKET_LENGTH, 0, (SOCKADDR*)&fromAddress, &fromAddressLength) != SOCKET_ERROR) {
			//Connectivity packet to filter out unwanted trafic.
			if (strcmp(packet, "CONNECTION\n") == 0) {
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
					if (!found)
					{
						m_clientAddresses.push_back(fromAddress);
						char ipbuf[INET_ADDRSTRLEN];
						printf("%s connected.\n", inet_ntop(AF_INET, &fromAddress, ipbuf, sizeof(ipbuf)));
					}
				}

				//Send connection packet back to client repeatedly in case it gets lost.
				sendto(m_socket, packet, PACKET_LENGTH, 0, (SOCKADDR*)&fromAddress, fromAddressLength);

				//Auto-start once we have 2+ clients. See commented code below for manual start implementation.
				if (m_clientAddresses.size() >= 2) {
					strcpy(packet, "START\n");
					for (size_t i = 0; i < m_clientAddresses.size(); i++) {
						sendto(m_socket, packet, PACKET_LENGTH, 0, (SOCKADDR*)&m_clientAddresses[i], fromAddressLength);
					}
					m_state = State::RUN;
					//run();
				}
				//\n is part of START because the input code appends \n.
				/*if (strcmp(packet, "START\n") == 0) {
					//Send the message back to the clients so they know to start.
					for (size_t i = 0; i < m_clientAddresses.size(); i++) {
						sendto(m_socket, packet, PACKET_LENGTH, 0, (SOCKADDR*)&m_clientAddresses[i], fromAddressLength);
					}
					run();
				}*/
			}
		}
	}
}

//1. Determine which client a message came from.
//2. Send that message to any client but the sender.
//3. Profit.
void Server::run()
{
	printf("Transmitting. . .\n");
	while (m_state == State::RUN) {
		SOCKADDR_IN fromAddress;
		memset(&fromAddress, 0, sizeof(fromAddress));
		int fromAddressLength = sizeof(fromAddress);

		char packet[PACKET_LENGTH];
		memset(packet, 0, PACKET_LENGTH);

		//Consider coming up with a condition to implicity shutdown ie m_clientAddresses.empty(); -> server broadcasts quit and shuts itself down.
		if (recvfrom(m_socket, packet, PACKET_LENGTH, 0, (SOCKADDR*)&fromAddress, &fromAddressLength) != SOCKET_ERROR) {
			if (strcmp(packet, "QUIT\n") == 0)
				shutdown();

			for (size_t i = 0; i < m_clientAddresses.size(); i++) {
				if (compareAddresses(fromAddress, m_clientAddresses[i]))
					continue;
				sendto(m_socket, packet, PACKET_LENGTH, 0, (SOCKADDR*)&m_clientAddresses[i], fromAddressLength);
			}
		}
	}
}

void Server::shutdown()
{
	char packet[PACKET_LENGTH] = "QUIT\n";
	for (size_t i = 0; i < m_clientAddresses.size(); i++) {
		sendto(m_socket, packet, PACKET_LENGTH, 0, (SOCKADDR*)&m_clientAddresses[i], sizeof(m_clientAddresses[i]));
	}
	closesocket(m_socket);
	cleanupWSA();
}

//Not sure if this is viable because more than one listener will consume packets meant for other listeners.
//If we were to make a listener exclusively for packets that wouldn't be terrible.
//We wouldn't be I/O bound regardless, but we might be able to bring clarity to our code by separating I/O from logic.
void Server::stateListener()
{
	while (true) {
		switch (m_state.load())
		{
		case CONNECT:
			break;
		case RUN:
			break;
		default:
			break;
		}
	}
}
