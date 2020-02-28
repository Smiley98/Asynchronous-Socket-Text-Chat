#include "Server.h"
#include <thread>
#include <future>
#include <cstdio>
#define SERVER_LOGGING true

void Server::start()
{
	if (!m_running) {
		initialize();
		m_running = true;
		std::thread(&Server::run, this).detach();
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
	case QUIT:
		printf("Quitting.\n");
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
		switch (getState())
		{
		case IDLE:
			break;
		case QUIT:
			break;
		case CONNECT:
			break;
		case DISCONNECT:
			break;
		case ROUTE:
			break;
		default:
			break;
		}
	}
}
