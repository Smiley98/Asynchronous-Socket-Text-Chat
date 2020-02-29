#define _CRT_SECURE_NO_WARNINGS
#include "Client.h"
#include "../Common/Timer.h"
#include <iostream>
#include <string>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);

	while (true) {
		std::string input;
		std::getline(std::cin, input);
		Packet packet(input, PacketMode::TWO_WAY);
		client.addOutgoing(packet);
	}

	return getchar();
}
