#include "Client.h"
#include "../Common/Timer.h"
#include "../Common/ClientInfo.h"
#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>

//Asynchronously receive and store keyboard input.
void pollInput(std::queue<std::string>& queue, std::mutex& mutex);

//Combine addresses with a byte vector representing a packet (including the packet's metadata).
Packet combine(const std::vector<Address>& addresses, const Packet& input, size_t size);

void appIdle() {

}

void appChat() {

}

void appGame() {

}

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);

	std::queue<std::string> inputQueue;
	std::mutex queueMutex;
	std::thread(pollInput, std::ref(inputQueue), std::ref(queueMutex)).detach();
	
	Timer networkTimer, renderTimer, gameTimer;
	PacketBuffer incoming;

	std::vector<Address> addresses;
	std::vector<ClientStatus> statuses;
	Address thisAddress;
	ClientStatus thisStatus = ClientStatus::FREE;

	while (true) {
		//Do network stuff every 0.1 seconds.
		if (networkTimer.elapsed() >= 100.0) {
			networkTimer.restart();
			client.copyIncoming(incoming);

			Packet thisAddressQuery(PacketType::THIS_CLIENT_ADDRESS, PacketMode::TWO_WAY);
			client.addOutgoing(thisAddressQuery);

			//Easier in the long run to handle all packets at once rather than search for specific packets.
			for (const Packet& packet : incoming) {
				switch (packet.getType())
				{
				case PacketType::THIS_CLIENT_ADDRESS:
					Packet::deserialize(packet, thisAddress);
					break;
				case PacketType::EVERY_CLIENT_ADDRESS:
					Packet::deserialize(packet, addresses);
					break;
				case PacketType::THIS_CLIENT_STATUS:
					Packet::deserialize(packet, thisStatus);
				case PacketType::EVERY_CLIENT_STATUS:
					Packet::deserialize(packet, statuses);
					break;
				default:
					break;
				}
			}
		}

		//Render every 100ms as well because the console has a terrible fill rate.
		if (renderTimer.elapsed() >= 100.0) {
			renderTimer.restart();
			system("cls");

			char clientLabel = 'A';
			for (const Address& address : addresses) {
				//if (clientInfo.first == thisClientInfo.first)
				//	continue;

				clientInfo.first.print();
				switch (clientInfo.second.m_status)
				{
				case FREE:
					printf("Client %c is free.\n", clientLabel);
					break;
				case IN_CHAT:
					printf("Client %c is in a chat.\n", clientLabel);
					break;
				case IN_GAME:
					printf("Client %c is in a game.\n", clientLabel);
					break;
				default:
					break;
				}
				clientLabel++;
			}
			printf("Type /g <client name> to start a game, /c <client name> to enter chat.\n");
		}

		//Run the game logic at 50fps.
		if (gameTimer.elapsed() >= 20.0) {
			gameTimer.restart();
			queueMutex.lock();
			while (inputQueue.size() > 0) {
				if (inputQueue.front().substr(0, 2) == "/g") {
					Packet command(PacketType::STATUS_UPDATE, PacketMode::MULTICAST);
					std::vector<Address> addresses(everyClientInfo.size()); 
					for (size_t i = 0; i < addresses.size(); i++) {
						addresses[i] = everyClientInfo[i].first;
					}
					combine(addresses, command,);
				}
				inputQueue.pop();
			}
			queueMutex.unlock();

			//Game logic:
			switch (thisClientInfo.second.m_status)
			{
			case FREE:
				appIdle();
				break;
			case IN_CHAT:
				appChat();
				break;
			case IN_GAME:
				appGame();
				break;
			default:
				break;
			}
		}
	}

	return getchar();
}

Packet combine(const std::vector<Address>& addresses, const Packet& input, size_t size) {
	assert(size < 256);
	assert(1 + sizeof(Address) * addresses.size() + 1 + size <= Packet::bufferSize());
	Packet output;
	Packet::serialize(addresses, output);
	const size_t dataStart = 1 + sizeof(Address) * addresses.size();
	output.write(&size, 1, dataStart);//Only one byte used to store the size so despite writing a size_t we can only store up to 256 bytes of tailing data.
	output.write(input.bytes(), size, dataStart + 1);
	return output;
	//We only need a function to aid in combining addresses with data for multicasting. The server does the extraction so we'll only ever receive data.
}

void pollInput(std::queue<std::string>& queue, std::mutex& mutex) {
	while (true) {
		std::string line;
		std::getline(std::cin, line);
		line += "\n";
		mutex.lock();
		queue.push(line);
		mutex.unlock();
	}
}