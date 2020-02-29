#include "Client.h"
#include <iostream>

int main() {
	Client client;
	client.start();
	client.setState(ClientState::CONSUME);
	return getchar();
}
