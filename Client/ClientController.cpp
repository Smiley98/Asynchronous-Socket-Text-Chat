#include "ClientController.h"
#include <thread>
#include <future>
#include <cstdio>
#define CLIENT_LOGGING true

void ClientController::start()
{
	if (!m_running) {
		m_client.initialize();
		std::thread(&run, this).detach();
		m_running = true;
	}
}

bool ClientController::running()
{
	return m_running;
}

ClientState ClientController::getState()
{
	return static_cast<ClientState>(m_state.load());
}

void ClientController::setState(ClientState clientState)
{
	m_state.store(clientState);
#ifdef CLIENT_LOGGING
	switch (clientState)
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
	case CONSUME:
		printf("Consuming.\n");
		break;
	default:
		break;
	}
#endif
}

void ClientController::run()
{
	while (m_running) {
		switch (getState()) {
		case IDLE:
			break;
		case QUIT:
			m_client.shutdown();
			m_running = false;
			break;
		case CONNECT: {
			if (m_client.exchange(PacketType::CONNECT))
				setState(CONSUME);
			break;
		}
		case DISCONNECT: {
			if (m_client.exchange(PacketType::DISCONNECT))
				setState(QUIT);
			break;
		}
		//Sends/receives all incoming/outgoing packets, leaving the client free to enque/deque packets at will!
		case CONSUME:
			std::future<void> syncSend = std::async(&Client::sendAll, &m_client, 0, true);
			std::future<void> syncRecv = std::async(&Client::recvAll, &m_client, 0, true);
			syncSend.wait();
			syncRecv.wait();
			break;
		}
	}
}
