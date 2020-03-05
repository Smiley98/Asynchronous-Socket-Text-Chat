#include "Client.h"
#include "../Common/Timer.h"
#include "../Common/ClientInfo.h"
#include "../Common/NetworkObject.h"
#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>

//Asynchronously receive and store keyboard input.
void pollInput(std::queue<std::string>& queue, std::mutex& mutex);

template<typename T>
Packet combine(const std::vector<Address>& addresses, const T& object, PacketType packetType, PacketMode packetMode = PacketMode::ONE_WAY) {
	//The packet type at the start doesn't matter because the data after the addresses is what's multicasted.
	Packet packet(PacketType::GENERIC, PacketMode::MULTICAST);
	Packet::serialize(addresses, packet);
	const size_t dataStart = 1 + sizeof(Address) * addresses.size();
	//Write the number of trailing bytes.
	packet.buffer()[dataStart] = sizeof(T) + Packet::headerSize();
	packet.buffer()[dataStart + 1] = static_cast<byte>(packetType);
	packet.buffer()[dataStart + 2] = static_cast<byte>(packetMode);
	//After trailing bytes + metadata, write the object.
	packet.write(&object, sizeof(object), dataStart + Packet::headerSize() + 1);
	return packet;
}

void appIdle() {

}

void appChat() {

}

void appGame() {

}

struct Test {
	int a = 6;
	double garbage;
	char moreGarbage[3];
	int b = 9;
};

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);

	std::queue<std::string> inputQueue;
	std::mutex queueMutex;
	std::thread(pollInput, std::ref(inputQueue), std::ref(queueMutex)).detach();
	
	Timer networkTimer, renderTimer, gameTimer;
	PacketBuffer incoming;

	std::vector<ClientInformation> allClientInfomration;
	ClientInformation thisClientInformation;

	//We never need more than one packet to read/write to/from because read/write operations are sequential copying.
	Packet packet(PacketType::GENERIC, PacketMode::ONE_WAY);

	while (true) {
		//Do network stuff every 0.1 seconds.
		if (networkTimer.elapsed() >= 100.0) {
			networkTimer.restart();
			client.copyIncoming(incoming);

			packet = Packet(PacketType::GET_THIS_CLIENT_INFORMATION, PacketMode::TWO_WAY);
			client.addOutgoing(packet);

			//Works
			//packet = Packet(PacketType::POSITION, PacketMode::BROADCAST);//Temporarily broadcast for testing.
			//Position pos{ 6, 9 };
			//Packet::serialize(pos, packet);
			//client.addOutgoing(packet);



			//Deserialize all incoming packets.
			for (const Packet& i : incoming) {
				switch (i.getType())
				{
				case PacketType::GET_ALL_CLIENT_INFORMATION:
					Packet::deserialize(i, allClientInfomration);
					break;
				case PacketType::GET_THIS_CLIENT_INFORMATION:
					Packet::deserialize(i, thisClientInformation);
					break;
				case PacketType::POSITION: {
					Position position;
					Packet::deserialize(i, position);
					printf("Client received: %s %hu %hu", i.typeString().c_str(), position.x, position.y);
					break;
				}
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
			for (const ClientInformation& clientInformation : allClientInfomration) {
				if (clientInformation.m_address == thisClientInformation.m_address)
					continue;
				clientInformation.m_address.print();

				switch (clientInformation.m_status)
				{
				case ClientStatus::FREE:
					printf("Client %c is free.\n", clientLabel);
					break;
				case ClientStatus::IN_CHAT:
					printf("Client %c is in a chat.\n", clientLabel);
					break;
				case ClientStatus::IN_GAME:
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
				//TODO: implement a mapping between client indices and labels so we can parse the clients we wish to multicast to ie A, B, C.
				if (inputQueue.front().substr(0, 2) == "/g") {//(Put every client in a game as of now).
					for (ClientInformation clientInformation : allClientInfomration) {
						packet = Packet(PacketType::SET_CLIENT_STATUS, PacketMode::ONE_WAY);
						clientInformation.m_status = ClientStatus::IN_GAME;
						Packet::serialize(clientInformation, packet);
						client.addOutgoing(packet);
					}
				}
				inputQueue.pop();
			}
			queueMutex.unlock();

			//Game logic:
			switch (thisClientInformation.m_status)
			{
			case ClientStatus::FREE:
				appIdle();
				break;
			case ClientStatus::IN_CHAT:
				appChat();
				break;
			case ClientStatus::IN_GAME:
				appGame();
				break;
			default:
				break;
			}
		}
	}

	return getchar();
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