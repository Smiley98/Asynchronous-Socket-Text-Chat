#include "Client.h"
#include "../Common/Timer.h"
#include "../Common/ClientInfo.h"
#include "../Common/NetworkObject.h"
#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <Windows.h>
#define rows 17
#define cols 11

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

void setCursor(short x, short y) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x, y });
}

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);

	std::queue<std::string> inputQueue;
	std::mutex queueMutex;
	std::thread(pollInput, std::ref(inputQueue), std::ref(queueMutex)).detach();
	
	Timer networkTimer, renderTimer;
	PacketBuffer incoming;

	std::vector<ClientInformation> allClientInfomration;
	ClientInformation thisClientInformation;

	//We never need more than one packet to read/write to/from because read/write operations are sequential copying.
	Packet packet(PacketType::GENERIC, PacketMode::ONE_WAY);

	Point player1, player2;
	Puck puck;

	unsigned char screen[rows][cols];
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			screen[i][j] = ' ';
		}
	}

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

			player1.x = 69;
			player1.y = 96;

			player2.x = 420;
			player2.y = 024;

			puck.position = player1;
			puck.velocity = player2;

			packet = Packet(PacketType::PLAYER, PacketMode::BROADCAST);
			Packet::serialize(player1, packet);
			client.addOutgoing(packet);

			packet = Packet(PacketType::PLAYER, PacketMode::BROADCAST);
			Packet::serialize(player2, packet);
			client.addOutgoing(packet);

			packet = Packet(PacketType::PUCK, PacketMode::BROADCAST);
			Packet::serialize(puck, packet);
			client.addOutgoing(packet);

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
				case PacketType::PLAYER: {
					Point data;
					Packet::deserialize(i, data);
					printf("Client received: %s %hu %hu\n", i.typeString().c_str(), data.x, data.y);
					break;
				}
				case PacketType::PUCK: {
					Puck data;
					Packet::deserialize(i, data);
					printf("Client received: %s %hu %hu %hu %hu\n", i.typeString().c_str(), data.position.x, data.position.y, data.velocity.x, data.velocity.y);
					break;
				}
				default:
					break;
				}
			}
		}

		if (renderTimer.elapsed() >= 100.0) {
			renderTimer.restart();
			system("cls");
			//setCursor(4, 5);
			//printf("X");

			for (int i = 0; i < rows; i++) {
				for (int j = 0; j < cols; j++) {
					printf("%c", screen[i][j]);
				}
				printf("\n");
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