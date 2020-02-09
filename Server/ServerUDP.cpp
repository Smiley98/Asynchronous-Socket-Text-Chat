#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")
#define BUFFER_LENGTH 512

void pollInput(std::queue<std::string>& queue, std::mutex& mutex) {
	while (true) {
		printf("Type to chat.\n");
		std::string line;
		std::getline(std::cin, line);
		line += "\n";
		mutex.lock();
		queue.push(line);
		mutex.unlock();
	}
}

int main() {
	// Initialize winsock
	WSADATA wsa;
	int error;
	error = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (error != 0) {
		printf("Failed to initialize %i\n", error);
		return 1;
	}

	// Create a Server socket
	addrinfo* addressInfo = NULL, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, "8888", &hints, &addressInfo) != 0) {
		printf("Getaddrinfo failed! %i\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	SOCKET soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (soc == INVALID_SOCKET) {
		printf("Failed creating a socket %i\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Bind socket
	if (bind(soc, addressInfo->ai_addr, (int)addressInfo->ai_addrlen) == SOCKET_ERROR) {
		printf("Bind failed: %i\n", WSAGetLastError());
		closesocket(soc);
		freeaddrinfo(addressInfo);
		WSACleanup();
		return 1;
	}

	// 0 for blocking, 1 for non-blocking
	u_long mode = 1;
	ioctlsocket(soc, FIONBIO, &mode);

	sockaddr_in fromAddr;
	memset(&fromAddr, 0, sizeof(fromAddr));
	int fromlen = sizeof(fromAddr);

	std::queue<std::string> messageQueue;
	std::mutex queueMutex;
	std::thread(pollInput, std::ref(messageQueue), std::ref(queueMutex)).detach();

	char messageBuffer[BUFFER_LENGTH];
	UINT counter = 0;

	// Message loop
	while (true) {
		//Receive messages if they exist.
		if (recvfrom(soc, messageBuffer, BUFFER_LENGTH, 0, (sockaddr*)&fromAddr, &fromlen) != SOCKET_ERROR) {
			//Very useful. IP is unique even for clients on the same computer!!!
			//char ipbuf[INET_ADDRSTRLEN];
			//printf("IP address: %s\n", inet_ntop(AF_INET, &fromAddr, ipbuf, sizeof(ipbuf)));
			printf("Received: %s\n", messageBuffer);
		}
		 
		//Send all queued messages.
		queueMutex.lock();
		while (messageQueue.size() > 0) {
			strcpy(messageBuffer, messageQueue.front().c_str());
			if (sendto(soc, messageBuffer, BUFFER_LENGTH, 0, (sockaddr*)&fromAddr, fromlen) == SOCKET_ERROR) {
				printf("Server send failed! %i\n", WSAGetLastError());
				getchar();
			}
			messageQueue.pop();
		}
		queueMutex.unlock();
	}

	//Shutdown the socket
	if (shutdown(soc, SD_BOTH) == SOCKET_ERROR) {
		printf("Shutdown failed! %i\n", WSAGetLastError());
		closesocket(soc);
		WSACleanup();
		return 1;
	}

	closesocket(soc);
	freeaddrinfo(addressInfo);
	WSACleanup();

	return 0;
}

//See the IP that a message came from:
//char ipbuf[INET_ADDRSTRLEN];
//printf("IP address: %s\n", inet_ntop(AF_INET, &fromAddr, ipbuf, sizeof(ipbuf)));