#include "Server.h"
#include <iostream>
int main() {
	Server server;
	server.start();
	server.setState(ServerState::ROUTE);
	return getchar();
}