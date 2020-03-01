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

			//Gotta have some not immediately relevant stuff here because of scope:
			std::vector<Address> addresses;
			std::vector<size_t> statusIndices;
			char clientLabel = 'A';

			std::vector<size_t> indices = findPacketOfType(PacketType::LIST_ALL_ACTIVE, incoming);
			if (indices.size() > 0) {
				//Have a printout of all the active clients, then switch state based on input ie /c followed by client names to chat, /g to start a game!
				Packet& recentInfo = incoming[indices.back()];
				addresses = Address::decode(recentInfo);
				//Packet data = decouple(recentInfo, addresses);//Decided not to go with a monolithic update packet.
				statusIndices = findPacketOfType(PacketType::STATUS_UPDATE, incoming);
				if (statusIndices.size() > 0) {
					system("cls");
					//This is less than ideal, but status update and indices should be 1:1.
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

			//Status packet.
			Packet p(PacketType::STATUS_UPDATE, PacketMode::MULTICAST);
			ClientStatus s = ClientStatus::FREE;
			queueMutex.lock();
			while (inputQueue.size() > 0) {
				//I am sorry about this logic xD.
				if (inputQueue.front()[0] == '/' && inputQueue.front()[1] == 'g') {
					if (inputQueue.front()[3] >= 'A' && inputQueue.front()[3] <= 'Z') {
						s = ClientStatus::IN_GAME;
						printf("Starting game with client %c.\n", inputQueue.front()[3]);
						if (addresses.size() > 0) {
							//Package the address and the status update data into the packet and call it a day.
							std::vector<Address> address = { addresses[inputQueue.front()[3] - clientLabel] };
							std::vector<byte> data{ s };
							p = combine(address, data);
						}
					}
				}
				//Didn't have enough time to actually implement proper client state and make a chat.
				else if (inputQueue.front()[0] == '/' && inputQueue.front()[1] == 'c') {
					if (inputQueue.front()[3] >= 'A' && inputQueue.front()[3] <= 'Z') {
						s = ClientStatus::IN_CHAT;
						printf("Chatting with client %c.\n", inputQueue.front()[3]);
						if (addresses.size() > 0) {
							//Package the address and the status update data into the packet and call it a day.
							std::vector<Address> address = { addresses[inputQueue.front()[3] - clientLabel] };
							std::vector<byte> data{ s };
							p = combine(address, data);
						}
					}
				}
				inputQueue.pop();
			}
			queueMutex.unlock();

			//Don't multicast if we're free.
			if (s == ClientStatus::FREE) {
				p.setMode(PacketMode::ONE_WAY);
				p.write(&s, 1);
			}
			client.addOutgoing(p);
		}
	}

	return getchar();
}
