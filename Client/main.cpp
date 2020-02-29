#include "Client.h"
#include <iostream>
#include <string>

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);

	while (true) {
		std::string input;
		std::getline(std::cin, input);
		Packet packet(input);
		client.addOutgoing(packet);
	}

	return getchar();
}
