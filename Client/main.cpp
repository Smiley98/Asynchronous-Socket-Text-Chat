#include "Client.h"
#include "../Common/Timer.h"
#include "../Common/Address.h"
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

/*Packet combine(const std::vector<Address>& addresses, const std::vector<byte>& data) {
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
}*/

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
			client.copyIncoming(incoming);

			queueMutex.lock();
			while (inputQueue.size() > 0) {
				inputQueue.front();
				inputQueue.pop();
			}
			queueMutex.unlock();
		}
	}

	return getchar();
}
