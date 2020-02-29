#define _CRT_SECURE_NO_WARNINGS
#include "Client.h"
#include "../Common/Timer.h"
#include <iostream>
#include <string>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

int main() {
	/*Client client;
	client.start();
	client.setState(ClientState::CONSUME);*/

	Network::initialize();
	SOCKET soc = Network::createSocket();
	ADDRINFO* address = Network::createAddress();
	Packet packet;
	const char* msg = "lit!";
	strcpy(packet.signedBytes(), msg);

	Timer timer;
	size_t counter = 0;
	while (true) {
		if (timer.elapsed() >= 1000.0) {
			sendto(soc, packet.signedBytes(), packet.size(), 0, address->ai_addr, address->ai_addrlen);
			timer.restart();
		}

		if (recvfrom(soc, packet.signedBytes(), packet.size(), 0, NULL, NULL) != SOCKET_ERROR) {
			printf("Client received%zu: %s\n", ++counter, packet.signedBytes());
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
