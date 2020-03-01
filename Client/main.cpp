#include "Client.h"
#include "../Common/Timer.h"
#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>

//Asynchronously receive and store keyboard input.
void pollInput(std::queue<std::string>& queue, std::mutex& mutex) {
	while (true) {
		printf("Type to chat.\n");
		std::string line;
		std::getline(std::cin, line);
		line += "\n";
		mutex.lock();
		queue.push(line);
		mutex.unlock();
	}
}

Packet combine(const std::vector<Address>& addresses, const std::vector<byte>& data) {
	Packet packet = Address::encode(addresses);
	//const size_t addressMemoryLength = 1 + addresses.size() * sizeof(Address);
	packet.write(data.data(), data.size(), 1 + addresses.size() * sizeof(Address));
	return packet;
}

//Retrieve the data from an address/data packet
Packet decouple(const Packet& packet, const std::vector<Address>& addresses) {
	size_t dataStart = 1 + addresses.size() * sizeof(Address);
	Packet result;
	packet.read(result.bytes(), Packet::bufferSize() - dataStart, dataStart);
	return result;
}

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);

	std::queue<std::string> inputQueue;
	std::mutex queueMutex;
	std::thread(pollInput, std::ref(inputQueue), std::ref(queueMutex)).detach();
	
	Timer timer;
	PacketBuffer incoming;
	while (true) {
		//Run at 10 fps cause why not :D
		if (timer.elapsed() >= 100.0) {
			timer.restart();
			//system("cls");//Must be dropping packets or something because the text flickers if I clear regardless. Will try only clearing on status change.
			client.copyIncoming(incoming);
			//Reassigned later (A + addresses.size() lmao).
			size_t clientCount = 0;

			//Could also write the client's status in these active list packets (ie available, in chat, in game).
			std::vector<size_t> indices = findPacketOfType(PacketType::LIST_ALL_ACTIVE, incoming);
			if (indices.size() > 0) {
				//Have a printout of all the active clients, then switch state based on input ie /c followed by client names to chat, /g to start a game!
				Packet& recentInfo = incoming[indices.back()];
				std::vector<Address> addresses = Address::decode(recentInfo);
				//Packet data = decouple(recentInfo, addresses);//Decided not to go with a monolithic update packet.
				std::vector<size_t> statusIndices = findPacketOfType(PacketType::STATUS_UPDATE, incoming);
				if (statusIndices.size() > 0) {
					system("cls");
					//This is less than ideal, but status update and indices should be 1:1.

					char clientLabel = 'A';
					for (size_t i = 0; i < addresses.size(); i++) {
						byte status = incoming[statusIndices.back()].buffer()[i];//Down the rabit hole. Smh...
						switch (status)
						{
						case FREE:
							printf("Client %c is free.\n", clientLabel);
							break;
						case IN_CHAT:
							printf("Client %c is in chat.\n", clientLabel);
							break;
						case IN_GAME:
							printf("Client %c is in game.\n", clientLabel);
							break;
						default:
							break;
						}
						clientLabel++;
					}
				}
			}

			queueMutex.lock();
			//Status packet.
			Packet p(PacketType::STATUS_UPDATE, PacketMode::ONE_WAY);
			ClientStatus s = ClientStatus::FREE;
			while (inputQueue.size() > 0) {
				if (inputQueue.front() == "/g") {
					printf("PRIMEOPS!!!\n");
					//Idk how to tell other clients that they're in game.
					s = ClientStatus::IN_GAME;
				}
				inputQueue.pop();
			}
			queueMutex.unlock();
			p.write(&s, 1);
			client.addOutgoing(p);
		}
	}

	return getchar();
}
