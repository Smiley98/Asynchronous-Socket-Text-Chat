#include "Client.h"
#include "../Common/Timer.h"
#include "../Common/ClientInfo.h"
#include "../Common/NetworkObjects.h"
#include "../Common/Multicast.h"
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
#undef max
#undef min

void pollInput(std::queue<std::string>& queue, std::mutex& mutex);
void setCursor(short x, short y);
void reset(unsigned char screen[rows][cols]);
void init(unsigned char screen[rows][cols]);
void render(unsigned char screen[rows][cols]);

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);

	std::queue<std::string> inputQueue;
	std::mutex queueMutex;
	std::thread(pollInput, std::ref(inputQueue), std::ref(queueMutex)).detach();
	
	Timer networkTimer, updateTimer;
	PacketBuffer incoming;

	std::vector<ClientInformation> allClientInfomration;
	ClientInformation thisClientInformation;

	Packet packet(PacketType::GENERIC, PacketMode::ONE_WAY);
	Puck puck{ 5, 9, -1, -1 };
	Point player1{ 5, 1 };
	Point player2{ 5, 15 };

	//Indicates whether this client updates the other client or vice-versa.
	bool master = false;

	unsigned char screen[rows][cols];
	init(screen);
	reset(screen);

	while (true) {
		//Do network stuff every 0.1 seconds.
		if (networkTimer.elapsed() >= 1000.0) {
			networkTimer.restart();
			client.copyIncoming(incoming);

			packet = Packet(PacketType::THIS_CLIENT_INFORMATION, PacketMode::TWO_WAY);
			client.addOutgoing(packet);

			//Deserialize all incoming packets.
			for (const Packet& i : incoming) {
				switch (i.getType())
				{
				case PacketType::ALL_CLIENT_INFORMATION: {
					Packet::deserialize(i, allClientInfomration);
					break;
				}
				case PacketType::THIS_CLIENT_INFORMATION: {
					Packet::deserialize(i, thisClientInformation);
					break;
				}
				case PacketType::OPPONENT_POSITION: {
					Point position;
					Packet::deserialize(i, position);
					if (master)
						player2 = position;
					else
						player1 = position;
					break;
				}
				default:
					break;
				}
			}
		}

		if (updateTimer.elapsed() >= 100.0) {
			updateTimer.restart();

			//Reset then copy game objects to buffer.
			reset(screen);
			screen[player1.y][player1.x] = playersymbol;
			screen[player2.y][player2.x] = playersymbol;
			screen[puck.position.y][puck.position.x] = pucksymbol;
			bool puckPositionUpdate = false;
			bool puckVelocityUpdate = false;

			//Player collision.
			if (screen[puck.position.y + puck.velocity.y][puck.position.x + puck.velocity.x] == playersymbol) {
				puck.velocity.x *= -1;
				puck.velocity.y *= -1;
				if (master)
					puckVelocityUpdate = true;
			}

			//Border collision.
			short puckFutureX = puck.position.x + puck.velocity.x;
			short puckFutureY = puck.position.y + puck.velocity.y;
			if (puckFutureX <= 0 || puckFutureX >= cols - 1) {
				puck.velocity.x *= -1;
				if (master)
					puckVelocityUpdate = true;
			}
			if (puckFutureY <= 0 || puckFutureY >= rows - 1) {
				puck.velocity.y *= -1;
				if (master)
					puckVelocityUpdate = true;
			}

			//No more puck logic, apply velocity.
			puck.position.y += puck.velocity.y;
			puck.position.x += puck.velocity.x;

			if(puckVelocityUpdate) {
				packet = Packet(PacketType::PUCK_VELOCITY, PacketMode::REROUTE);
				Packet::serialize(puck.velocity, packet);
				client.addOutgoing(packet);
			}

			bool input = false;
			if (GetAsyncKeyState(VK_LEFT)) {
				input = true;
				if (master) {
					if (player1.x - 1 > 0)
						player1.x--;
				}
				else {
					if (player2.x - 1 > 0)
						player2.x--;
				}
			}
			else if (GetAsyncKeyState(VK_RIGHT)) {
				input = true;
				if (master) {
					if (player1.x + 1 < cols - 1)
						player1.x++;
				}
				else {
					if (player2.x + 1 < cols - 1)
						player2.x++;
				}
			}

			//Send the position of this client's player to the other client.
			if (input) {
				packet = Packet(PacketType::OPPONENT_POSITION, PacketMode::REROUTE);
				if (master)
					Packet::serialize(player1, packet);
				else
					Packet::serialize(player2, packet);
				client.addOutgoing(packet);
			}

			//Clear and render screen.
			//setCursor(0, 0);
			//render(screen);
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