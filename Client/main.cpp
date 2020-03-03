#include "Client.h"
#include "../Common/Timer.h"
#include "../Common/ClientInfo.h"
#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>

//Asynchronously receive and store keyboard input.
void pollInput(std::queue<std::string>& queue, std::mutex& mutex) {
	while (true) {
		//printf("Type to chat.\n");
		printf("Type /g <client name> to start a game, /c <client name> to enter chat.\n");
		std::string line;
		std::getline(std::cin, line);
		line += "\n";
		mutex.lock();
		queue.push(line);
		mutex.unlock();
	}
}

//Combine addresses with a byte vector representing a packet (including the packet's metadata).
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

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);

	std::queue<std::string> inputQueue;
	std::mutex queueMutex;
	std::thread(pollInput, std::ref(inputQueue), std::ref(queueMutex)).detach();
	
	Timer networkTimer, renderTimer, inputTimer;
	PacketBuffer incoming;
	std::vector<ClientInfo> clientInfo;

	while (true) {
		//Do network stuff every 0.1 seconds.
		if (networkTimer.elapsed() >= 100.0) {
			networkTimer.restart();
			client.copyIncoming(incoming);
			Packet ping(PacketType::GENERIC, PacketMode::ONE_WAY);
			client.addOutgoing(ping);

			//Update the client information only if there's new information.
			std::vector<size_t> clientInfoIndices = findPacketOfType(PacketType::ALL_CLIENT_INFORMATION, incoming);
			if (clientInfoIndices.size() > 0) {
				Packet& mostRecentClientInfo = incoming[clientInfoIndices.back()];
				Packet::deserialize(mostRecentClientInfo, clientInfo);
			}
		}

		//Render every 100ms as well because the console has a terrible fill rate.
		if (renderTimer.elapsed() >= 100.0) {
			renderTimer.restart();
			system("cls");
			char clientLabel = 'A';
			for (const ClientInfo& cl : clientInfo) {
				cl.first.print();
				switch (cl.second.m_status)
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
		}

		queueMutex.lock();
		while (inputQueue.size() > 0) {
			inputQueue.front();
			inputQueue.pop();
		}
		queueMutex.unlock();
	}

	return getchar();
}
