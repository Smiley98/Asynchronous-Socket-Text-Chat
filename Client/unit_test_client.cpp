#include "Client.h"
#include <iostream>
/*#include <string>
#include <thread>
#include <concurrent_queue.h>

void pollInput(concurrency::concurrent_queue<std::string>& queue) {
	while (true) {
		printf("Type to chat.\n");
		std::string line;
		std::getline(std::cin, line);
		line += "\n";
		queue.push(line);
	}
}*/

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);
	return getchar();
}