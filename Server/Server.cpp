#include "Server.h"
#include <thread>
#include <future>
#include <cstdio>
#define SERVER_LOGGING true

Server::Server()
{
	initialize();
}

Server::~Server()
{
	stop();
	shutdown();
}

void Server::start()
{
	if (!m_running) {
		m_running = true;
		m_thread = std::thread(&Server::run, this);
	}
}

void Server::stop()
{
	if (running()) {
		m_running = false;
		m_thread.join();
	}
}

bool Server::running()
{
	return m_running;
}

ServerState Server::getState()
{
	return static_cast<ServerState>(m_state.load());
}

void Server::setState(ServerState serverState)
{
	m_state.store(serverState);
#ifdef SERVER_LOGGING
	switch (serverState)
	{
	case IDLE:
		printf("Idling...\n");
		break;
	case CONNECT:
		printf("Connecting.\n");
		break;
	case DISCONNECT:
		printf("Disconnecting.\n");
		break;
	case ROUTE:
		printf("Routing.\n");
		break;
	default:
		break;
	}
#endif
}

void Server::run()
{
	while (m_running) {
		switch (getState()) {
		case IDLE:
			break;
		case CONNECT:
			break;
		case DISCONNECT:
			break;
		case ROUTE: {
			std::future<void> syncRecv = std::async(&Server::recvAll, this);
			std::future<void> syncSend = std::async(&Server::sendAll, this);
			syncRecv.wait();
			//Wait till we're done receiving before potentially disconnecting clients.
			std::future<void> syncRefresh = std::async(&Server::refresh, this);
			syncSend.wait();
			//Wait till we're done sending before clearing outgoing packets.
			std::future<void> syncTransfer = std::async(&Server::transfer, this);
			syncRefresh.wait();
			syncTransfer.wait();
			break;
		}
		default:
			break;
		}
	}
}
