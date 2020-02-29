#include "Server.h"
#include <iostream>
#include <string>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

int main() {
	//A: Initialize explicitly and manually specify addresses.
	/*
	Network::initialize();
	SOCKET soc = Network::createSocket();
	ADDRINFO* addressInfo = Network::createAddress(true, "");
	Network::bindSocket(soc, addressInfo);

	SOCKADDR_IN fromAddress;
	int fromLength = sizeof(fromAddress);
	Packet packet;
	while (true) {
		if (recvfrom(soc, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&fromAddress, &fromLength) != SOCKET_ERROR) {
			static size_t counter;
			printf("Server received%zu: %s\n", ++counter, packet.signedBytes());
			sendto(soc, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&fromAddress, fromLength);
		}
	}//*/

	//B: Use server, explicit addresses.
	///*
	Server server;
	Packet packet;
	SOCKADDR_IN fromAddress;
	int fromLength = sizeof(fromAddress);
	while (true) {
		if (recvfrom(server.m_socket, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&fromAddress, &fromLength) != SOCKET_ERROR) {
			static size_t counter;
			printf("Server received%zu: %s\n", ++counter, packet.signedBytes());
			sendto(server.m_socket, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&fromAddress, fromLength);
		}
	}//*/

	//C: Initialize explicitly, use addresses.
	/*
	Network::initialize();
	SOCKET soc = Network::createSocket();
	ADDRINFO* addressInfo = Network::createAddress(true, "");
	Network::bindSocket(soc, addressInfo);
	Address address;
	Packet packet;
	while (true) {
		if (recvfrom(soc, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&address.m_sai, &address.m_length) != SOCKET_ERROR) {
			static size_t counter;
			printf("Server received%zu: %s\n", ++counter, packet.signedBytes());
			sendto(soc, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&address.m_sai, address.m_length);
		}
	}//*/
	
	//D: Use both server and addresses.
	/*
	Server server;
	Address address;
	Packet packet;
	while (true) {
		if (recvfrom(server.m_socket, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&address.m_sai, &address.m_length) != SOCKET_ERROR) {
			static size_t counter;
			printf("Server received%zu: %s\n", ++counter, packet.signedBytes());
			sendto(server.m_socket, packet.signedBytes(), packet.size(), 0, (SOCKADDR*)&address.m_sai, address.m_length);
		}
	}//*/

	return getchar();
}

/*Server server;
server.start();
server.setState(ServerState::ROUTE);*/