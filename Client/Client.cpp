#include "Client.h"
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

void Client::init(const std::string& host)
{
	startupWSA();

	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	if (getaddrinfo(host.c_str(), "8888", &hints, &m_addressInfo) != 0) {
		printf("Address resolution failed (%i).\n", WSAGetLastError());
		shutdown();
	}

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET) {
		printf("Socket creation failed (%i).\n", WSAGetLastError());
		shutdown();
	}

	u_long mode = 1;
	if (ioctlsocket(m_socket, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("Socket I/O control failed (%i).\n", WSAGetLastError());
		shutdown();
	}
}

void Client::connect(bool& connected, bool& running)
{
	//while (true) {
		char packet[PACKET_LENGTH] = "CONNECTION\n";
		sendto(m_socket, packet, PACKET_LENGTH, 0, m_addressInfo->ai_addr, m_addressInfo->ai_addrlen);

		memset(packet, 0, PACKET_LENGTH);
		if (recvfrom(m_socket, packet, PACKET_LENGTH, 0, NULL, NULL) != SOCKET_ERROR) {
			if (strcmp(packet, "CONNECTION\n") == 0)
				connected = true;
			else if (strcmp(packet, "START\n") == 0)
				running = true;
				//run();
		}
	//}
}

void Client::send(const Packet& packet)
{	//Lightning fast! 
	sendto(m_socket, packet.data, PACKET_LENGTH, 0, m_addressInfo->ai_addr, m_addressInfo->ai_addrlen);
}

//Gotta come up with a server quit policy ie 0 clients (clients can send disconnect messages), and then server would shut itself down.
void Client::run()
{
	printf("Running. . .\n");
	while (true) {
		char packet[PACKET_LENGTH];
		memset(packet, 0, PACKET_LENGTH);
		if (recvfrom(m_socket, packet, PACKET_LENGTH, 0, NULL, NULL) != SOCKET_ERROR) {
			if (strcmp(packet, "QUIT\n") == 0)
				shutdown();

			m_packets.resize(m_packets.size() + 1);
			memcpy(m_packets.back().data, packet, PACKET_LENGTH);
		}
	}
}

concurrency::concurrent_vector<Packet>& Client::packets()
{
	return m_packets;
}

/*void Client::start()
{
	char packet[PACKET_LENGTH] = "START\n";
	sendto(m_socket, packet, PACKET_LENGTH, 0, m_addressInfo->ai_addr, m_addressInfo->ai_addrlen);
}*/

void Client::shutdown()
{
	closesocket(m_socket);
	freeaddrinfo(m_addressInfo);
	cleanupWSA();
}
