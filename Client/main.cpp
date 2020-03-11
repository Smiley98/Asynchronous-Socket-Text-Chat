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
#define LOGGING true

void pollInput(std::queue<std::string>& queue, std::mutex& mutex);
void setCursor(short x, short y);
void reset(unsigned char screen[rows][cols]);
void init(unsigned char screen[rows][cols]);
void render(unsigned char screen[rows][cols]);

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);

	//An artifact from assignment 1 ;)
	//std::queue<std::string> inputQueue;
	//std::mutex queueMutex;
	//std::thread(pollInput, std::ref(inputQueue), std::ref(queueMutex)).detach();
	
	Timer networkTimer, updateTimer, goalTimer, latencyTimer;
	PacketBuffer incoming;

	std::vector<ClientInformation> allClientInfomration;
	ClientInformation thisClientInformation;

	Packet packet(PacketType::GENERIC, PacketMode::ONE_WAY);
	const Point centre{ 5, 8 };
	Puck puck{ centre, -1, -1 };
	Point player1{ 5, 1 };
	Point player2{ 5, 15 };
	Point score{ 0, 0 };//unsigned short score1 = 0, score2 = 0;//Lol its easier to serialize a 2 component number despite being unintuitive.
	bool p1Goal = false, p2Goal = false;

	//Indicates whether this client updates the other client or vice-versa.
	bool master = false;
	//Sync status. Master sends to self (for an attempt at round-trip latency) and slave, slave just accepts.
	bool synced = false;

	//A hack to measure latency in attempt to sync.
	double latency = 0.0;
	bool measuringLatency = false;

	double lag = 0.0;

	unsigned char screen[rows][cols];
	init(screen);
	reset(screen);

	while (true) {
		//Do network stuff every 0.1 seconds.
		if (networkTimer.elapsed() >= 100.0 + lag) {
			networkTimer.restart();
			client.copyIncoming(incoming);

			packet = Packet(PacketType::THIS_CLIENT_INFORMATION, PacketMode::TWO_WAY);
			client.addOutgoing(packet);

			if (!measuringLatency) {
				packet = Packet(PacketType::LATENCY, PacketMode::TWO_WAY);
				client.addOutgoing(packet);
				measuringLatency = true;
				latencyTimer.restart();
			}

			//Deserialize all incoming packets.
			for (const Packet& i : incoming) {
#if LOGGING
				if (!i.typeString().empty()) {
					setCursor(cols, 0);
					printf("Client received packet of type %s.", i.typeString().c_str());
				}
#endif
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
				case PacketType::PUCK_POSITION: {
					Packet::deserialize(i, puck.position);
#if LOGGING
					setCursor(cols, 1);
					printf("Data: %hi, %hi.", puck.position.x, puck.position.y);
#endif
					break;
				}
				case PacketType::PUCK_VELOCITY: {
					Packet::deserialize(i, puck.velocity);
#if LOGGING
					setCursor(cols, 1);
					printf("Data: %hi, %hi.", puck.velocity.x, puck.velocity.y);
#endif
					break;
				}
				case PacketType::OPPONENT_POSITION: {
					Point position;
					Packet::deserialize(i, position);
					if (master)
						player2 = position;
					else
						player1 = position;
#if LOGGING
					setCursor(cols, 1);
					printf("Data: %hi, %hi.", position.x, position.y);
#endif
					break;
				}
				case PacketType::SYNC: {
					synced = true;
					break;
				}
				case PacketType::SCORE: {
					Packet::deserialize(i, score);
#if LOGGING
					setCursor(cols, 1);
					printf("Data: %hi, %hi.", score.x, score.y);
#endif
					break;
				}
				case PacketType::LATENCY: {
					latency = latencyTimer.elapsed();
					measuringLatency = false;
					break;
				}
				default:
					break;
				}
			}
			size_t lowest = std::numeric_limits<size_t>::max();
			for (const ClientInformation& clientInformation : allClientInfomration) {
				if (clientInformation.m_id < lowest)
					lowest = clientInformation.m_id;
			}
			if (thisClientInformation.m_id == lowest)
				master = true;
		}
		if (allClientInfomration.size() < 2) {
			printf("Waiting for other players. . .");
			system("cls");
			continue;
		}
		
		if (updateTimer.elapsed() >= 100.0) {
			updateTimer.restart();

			if (score.x >= 5) {
				system("cls");
				printf("Player one wins!");
				continue;
			}
			else if (score.y >= 5) {
				system("cls");
				printf("Player two wins!");
				continue;
			}

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
				if (puckFutureY <= 0)
					p2Goal = true;
				else if (puckFutureY >= rows - 1)
					p1Goal = true;
				puck.velocity.y *= -1;
				if (master)
					puckVelocityUpdate = true;
			}

			if (p1Goal || p2Goal) {
				puck.position = centre;
				packet = Packet(PacketType::PUCK_POSITION, PacketMode::REROUTE);
				Packet::serialize(puck.position, packet);
				client.addOutgoing(packet);
				
				//Idk why this doesn't work :( Maybe cause this game is frame-based xD.
				//The proper way to do lock-step would be with TCP sockets which I did not plan for and thus cannot reasonably support at this time.
				if (!synced) {
					packet = Packet(PacketType::SYNC, PacketMode::BROADCAST);
					client.addOutgoing(packet);
					continue;
				}

				if (p1Goal) {
					score.x++;
					puck.velocity.y = -1;
					p1Goal = false;
				}
				else if (p2Goal) {
					score.y++;
					puck.velocity.y = 1;
					p2Goal = false;
				}
				packet = Packet(PacketType::SCORE, PacketMode::REROUTE);
				Packet::serialize(score, packet);
				client.addOutgoing(packet);

				synced = false;
				goalTimer.restart();
			}
			
			//Try and account for latency. Not sure if this is considered a hack xD. Doesn't matter, it doesn't work...
			double goalDelay = master ? latency + 3000.0 : 3000.0;
			//printf("Travel time: %f\n", travelTime);
			if (goalTimer.elapsed() >= goalDelay) {
				puck.position.y += puck.velocity.y;
				puck.position.x += puck.velocity.x;
			}
			
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

			//Lag switches.
			if (GetAsyncKeyState(49)) {
				lag += 100.0;
				printf("New lag: %f.\n", lag);
			}
			else if (GetAsyncKeyState(50)) {
				lag -= 100.0;
				printf("New lag: %f.\n", lag);
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
			setCursor(0, 0);
			if (master)
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 128);
			render(screen);
			printf("Player 1: %hi/5, player 2: %hi/5.\n", score.x, score.y);
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