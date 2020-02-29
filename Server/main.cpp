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
	char data[64];

	//SOCKADDR_IN fromAddress;
	//int fromLength = sizeof(fromAddress);

	while (true) {
		if (recvfrom(soc, data, sizeof(data), 0, NULL, NULL) != SOCKET_ERROR) {
			printf("Received: %s\n", data);
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
