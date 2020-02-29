#include "Server.h"
#include <iostream>
#include <string>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

int main() {
	/*Server server;
	server.start();
	server.setState(ServerState::ROUTE);*/
	Network::initialize();
	SOCKET soc = Network::createSocket();
	ADDRINFO* address = Network::createAddress(true, "");
	Network::bindSocket(soc, address);

	SOCKADDR_IN fromAddress;
	int fromLength = sizeof(fromAddress);

	Packet packet;
	size_t counter = 0;
	while (true) {
		if (recvfrom(soc, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&fromAddress, &fromLength) != SOCKET_ERROR) {
			printf("Server received%zu: %s\n", ++counter, packet.signedBytes());
			sendto(soc, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&fromAddress, fromLength);
		}
	}

	/*while (true) {
		std::string input;
		std::getline(std::cin, input);
		Packet packet(input);
		client.addOutgoing(packet);
	}*/

	return getchar();
}
