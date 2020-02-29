#include "Client.h"
#include "../Common/Timer.h"
#include <iostream>
#include <string>
#include <thread>

void pollInput(Client& client) {
	while (true) {
		std::string input;
		std::getline(std::cin, input);
		//TODO: Add enhanced logic based on input ie /c to chat, /g to start game.
		Packet packet(input, PacketMode::TWO_WAY);
		client.addOutgoing(packet);
	}
}

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);
	std::thread(pollInput, std::ref(client)).detach();
	
	Timer timer;
	PacketBuffer incoming;
	while (true) {
		//Ask the server for data every 100ms.
		if (timer.elapsed() >= 100.0) {
			timer.restart();
			client.copyIncoming(incoming);
			std::vector<size_t> indices = findPacketOfType(PacketType::LIST_ALL_ACTIVE, incoming);
			if (indices.size() > 0) {
				//Have a printout of all the active clients, then switch state based on input ie /c followed by client names to chat, /g to start a game!
			}
			//Probably don't need to handle ALL incoming packets IMMEDIATELY upon receive.
			for (const Packet& packet : incoming) {
				if (packet.getType() == PacketType::STRING)
					printf("%s\n", packet.toString().c_str());
			}
		}
	}

	return getchar();
}
