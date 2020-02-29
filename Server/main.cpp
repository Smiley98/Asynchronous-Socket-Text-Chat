#include "Server.h"
#include <iostream>
#include <string>

int main() {
	Server server;
	server.start();
	server.setState(ServerState::ROUTE);

	while (true);

	/*while (true) {
		std::string input;
		std::getline(std::cin, input);
		Packet packet(input);
		client.addOutgoing(packet);
	}*/

	return getchar();
}
