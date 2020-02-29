#include "Server.h"

int main() {
	Server server;
	server.start();
	server.setState(ServerState::ROUTE);

	return getchar();
}
