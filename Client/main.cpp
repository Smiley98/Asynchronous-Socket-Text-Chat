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
#define playersymbol 'X'
#define pucksymbol 'O'

//Asynchronously receive and store keyboard input.
void pollInput(std::queue<std::string>& queue, std::mutex& mutex);

//template<typename T>
//Packet combine(const std::vector<Address>& addresses, const T& object, PacketType packetType, PacketMode packetMode = PacketMode::ONE_WAY) {
//	//The packet type at the start doesn't matter because the data after the addresses is what's multicasted.
//	Packet packet(PacketType::GENERIC, PacketMode::MULTICAST);
//	Packet::serialize(addresses, packet);
//	const size_t dataStart = 1 + sizeof(Address) * addresses.size();
//	//Write the number of trailing bytes.
//	packet.buffer()[dataStart] = sizeof(T) + Packet::headerSize();
//	packet.buffer()[dataStart + 1] = static_cast<byte>(packetType);
//	packet.buffer()[dataStart + 2] = static_cast<byte>(packetMode);
//	//After trailing bytes + metadata, write the object.
//	packet.write(&object, sizeof(object), dataStart + Packet::headerSize() + 1);
//	return packet;
//}

struct MulticastPacket {
	Packet m_packet;
	std::vector<Address> m_addresses;
};

//Forms a packet containing all addresses at the front followed by metadata and an object.
template<typename T>
Packet serialize(const std::vector<Address>& addresses, const T& object, PacketType packetType, PacketMode packetMode = PacketMode::ONE_WAY) {
	//1. Write the addresses to a multicasted packet.
	assert(addresses.size()) > 0;
	Packet packet(PacketType::GENERIC, PacketMode::MULTICAST);
	Packet::serialize(addresses, packet);
	size_t dataIndex = sizeof(short) + sizeof(Address) * addresses.size();

	//2. Write the object count.
	const short objectCount = 1;
	packet.write(&objectCount, sizeof(short), dataStart);
	dataIndex += sizeof(short);

	//3. Write the size of a single object.
	const short objectSize = sizeof(T);
	packet.write(&objectSize, sizeof(short), dataIndex);
	dataIndex += sizeof(short);

	//4. Write the metadata.
	packet.write(&packetType, sizeof(PacketType), dataIndex);
	dataIndex += sizeof(PacketType);
	packet.write(&packetMode, sizeof(PacketMode), dataIndex);
	dataIndex += sizeof(PacketMode);

	//5. Write the raw object.
	packet.write(&object, sizeof(object), dataIndex);
	
	return packet;
}

//Forms a packet containing all addresses at the front followed by metadata and objects.
template<typename T>
Packet serialize(const std::vector<Address>& addresses, const std::vector<T>& objects, PacketType packetType, PacketMode packetMode = PacketMode::ONE_WAY) {
	//1. Write the addresses to a multicasted packet.
	assert(addresses.size()) > 0;
	Packet packet(PacketType::GENERIC, PacketMode::MULTICAST);
	Packet::serialize(addresses, packet);
	size_t dataIndex = sizeof(short) + sizeof(Address) * addresses.size();

	//2. Write the number of objects.
	assert(objects.size() > 0);
	const unsigned short objectCount = objects.size();
	packet.write(&objectCount, sizeof(short), dataIndex);
	dataIndex += sizeof(short);

	//3. Write the size (in bytes) of a single object.
	const unsigned short objectSize = sizeof(T);
	packet.write(&objectSize, sizeof(short), dataIndex);
	dataIndex += sizeof(short);

	//4. Extract packet metadata.
	packet.write(&packetType, sizeof(PacketType), dataIndex);
	dataIndex += sizeof(PacketType);
	packet.write(&packetMode, sizeof(PacketMode), dataIndex);
	dataIndex += sizeof(PacketMode);

	//5. Extract packet object data.
	for (unsigned short i = 0; i < objectCount; i++) {
		packet.write(objects[i], sizeof(T), dataIndex);
		dataIndex += sizeof(T);
	}
	
	return packet;
}

MulticastPacket deserialize(const Packet& packet) {
	//1. Extract address information.
	std::vector<Address> addresses;
	Packet::deserialize(packet, addresses);
	assert(addresses.size() > 0);
	size_t dataIndex = sizeof(short) + sizeof(Address) * addresses.size();

	//2. Extract object count.
	unsigned short objectCount = 0;
	packet.read(&objectCount, sizeof(short), dataIndex);
	assert(objectCount > 0);
	dataIndex += sizeof(short);

	//3. Extract object size.
	unsigned short objectSize = 0;
	packet.read(&objectSize, sizeof(short), dataIndex);
	assert(objectSize > 0);
	dataIndex += sizeof(short);

	//4. Extract packet metadata.
	PacketType packetType;
	packet.read(&packetType, sizeof(PacketType), dataIndex);
	dataIndex += sizeof(PacketType);
	PacketMode packetMode;
	packet.read(&packetMode, sizeof(PacketMode), dataIndex);
	dataIndex += sizeof(PacketMode);

	//5. Extract packet object data.
	Packet outgoing(packetType, packetMode);
	outgoing.write(&packet.buffer(), objectSize * objectCount, dataIndex);

	return { outgoing, addresses };
}

void setCursor(short x, short y) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { x, y });
}

void reset(unsigned char screen[rows][cols]) {
	//Zero everything excluding borders.
	for (int i = 1; i < rows - 1; i++) {
		for (int j = 1; j < cols - 1; j++) {
			screen[i][j] = ' ';
		}
	}
}

void init(unsigned char screen[rows][cols]) {
	//Horizontal borders.
	for (int i = 0; i < rows; i++) {
		screen[i][0] = '|';
		screen[i][cols - 1] = '|';
	}
	//Vertical borders.
	for (int i = 0; i < cols; i++) {
		screen[0][i] = '-';
		screen[rows - 1][i] = '-';
	}
}

void render(unsigned char screen[rows][cols]) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			printf("%c", screen[i][j]);
		}
		printf("\n");
	}
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

	Puck puck{ 5, 9, -1, -1 };
	Point player1{ 5, 1 };
	Point player2{ 5, 15 };
	bool isPlayerOne;// , bothConnected = false;

	unsigned char screen[rows][cols];
	init(screen);

	while (true) {
		//Do network stuff every 0.1 seconds.
		if (networkTimer.elapsed() >= 100.0) {
			networkTimer.restart();
			client.copyIncoming(incoming);

			packet = Packet(PacketType::GET_THIS_CLIENT_INFORMATION, PacketMode::TWO_WAY);
			client.addOutgoing(packet);

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
					printf("Client received: %s %h %h\n", i.typeString().c_str(), data.x, data.y);
					break;
				}
				case PacketType::PUCK: {
					Puck data;
					Packet::deserialize(i, data);
					printf("Client received: %s %h %h %h %h\n", i.typeString().c_str(), data.position.x, data.position.y, data.velocity.x, data.velocity.y);
					break;
				}
				default:
					break;
				}
			}

			//Only determine the players once and only do so once we're guaranteed to have enough information.
			if (allClientInfomration.size() >= 2 && thisClientInformation.m_id > 0) {
				static bool once = true;
				if (once) {
					once = false;
					for (const ClientInformation& clientInformation : allClientInfomration)
						isPlayerOne = thisClientInformation.m_id <= clientInformation.m_id;
				}
			}
		}

		if (renderTimer.elapsed() >= 100.0) {
			renderTimer.restart();
			//system("cls");//Much faster to reposition the cursor instead of clearing the screen.
			setCursor(0, 0);
			reset(screen);
			screen[player1.y][player1.x] = playersymbol;
			screen[player2.y][player2.x] = playersymbol;
			screen[puck.position.y][puck.position.x] = pucksymbol;
			//Player collision.
			if (screen[puck.position.y + puck.velocity.y][puck.position.x + puck.velocity.x] == playersymbol) {
				puck.velocity.x *= -1;
				puck.velocity.y *= -1;
			}
			//Border collision.
			short futureX = puck.position.x + puck.velocity.x;
			short futureY = puck.position.y + puck.velocity.y;
			if (futureX <= 0 || futureX >= cols - 1)
				puck.velocity.x = -puck.velocity.x;
			if (futureY <= 0 || futureY >= rows - 1)
				puck.velocity.y = -puck.velocity.y;

			//I didn't have enough time to assign players via network.
			if (GetAsyncKeyState(VK_LEFT)) {
				if (player1.x - 1 > 0)
					player1.x--;
			}
			else if (GetAsyncKeyState(VK_RIGHT)) {
				if (player1.x + 1 < cols - 1)
					player1.x++;
			}

			puck.position.y += puck.velocity.y;
			puck.position.x += puck.velocity.x;

			render(screen);
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