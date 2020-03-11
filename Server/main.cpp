#include "Server.h"
#include "../Common/Address.h"
#include <iostream>
#include <Windows.h>

int main() {
	Server server;
	server.start();
	server.setState(ServerState::ROUTE);

	//3 and 4 on the server because GetAsyncKeyState isn't application-specific. Also its extremely sensitive.
	while (true) {
		if (GetAsyncKeyState(51))
			server.addLag(0.1);
		if (GetAsyncKeyState(52))
			server.addLag(-0.1);
	}

	return getchar();
}
