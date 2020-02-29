#define _CRT_SECURE_NO_WARNINGS
#include "Client.h"
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
	char data[64];
	const char* msg = "lit!";
	strcpy(data, msg);

	while (true) {
		sendto(soc, data, sizeof(data), 0, address->ai_addr, address->ai_addrlen);
		//Don't worry about receiving yet.
	}

	/*while (true) {
		std::string input;
		std::getline(std::cin, input);
		Packet packet(input);
		client.addOutgoing(packet);
	}*/

	return getchar();
}
