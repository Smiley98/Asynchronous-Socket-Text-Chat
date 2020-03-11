#include "Server.h"
#include "../Common/Address.h"
#include <iostream>

int main() {
	Server server;
	server.start();
	server.setState(ServerState::ROUTE);

	//These are different!
	Address a1;
	a1.m_sai.sin_addr.s_addr = 16777343;
	a1.m_sai.sin_port = 35582;
	Address a2;
	a2.m_sai.sin_addr.s_addr = 16777343;
	a2.m_sai.sin_port = 35326;

	printf("%llu %llu\n", hashAddress(a1), hashAddress(a2));

	return getchar();
}
