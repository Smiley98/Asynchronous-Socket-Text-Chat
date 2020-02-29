#include "Server.h"
#include <iostream>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

int main() {
	/*Server server;
	server.start();
	server.setState(ServerState::ROUTE);*/

	Server server;
	Address address;
	Packet packet;
	while (true) {
		if (recvfrom(server.m_socket, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&address.m_sai, &address.m_length) != SOCKET_ERROR) {
			static size_t counter;
			printf("Server received%zu: %s\n", ++counter, packet.signedBytes());
			sendto(server.m_socket, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&address.m_sai, address.m_length);
		}
	}

	return getchar();
}
