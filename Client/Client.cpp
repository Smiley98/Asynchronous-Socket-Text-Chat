#include "Client.h"
#include <thread>
#include <future>
#include <cstdio>
#define CLIENT_LOGGING true

void Client::start()
{
	if (!m_running) {
		initialize();
		std::thread(&run, this).detach();
		m_running = true;
	}
}

bool Client::running()
{
	return m_running;
}

ClientState Client::getState()
{
	return static_cast<ClientState>(m_state.load());
}

void Client::setState(ClientState clientState)
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

void Client::run()
{
	while (m_running) {
		switch (getState()) {
		case IDLE:
			break;
		case QUIT:
			shutdown();
			m_running = false;
			break;
		case CONNECT: {
			if (exchange(PacketType::CONNECT))
				setState(CONSUME);
			break;
		}
		case DISCONNECT: {
			if (exchange(PacketType::DISCONNECT))
				setState(QUIT);
			break;
		}
		//Sends/receives all incoming/outgoing packets, leaving the client free to enque/deque packets at will!
		case CONSUME:
			std::future<void> syncSend = std::async(&Client::sendAll, this, 0, true);
			std::future<void> syncRecv = std::async(&Client::recvAll, this, 0, true);
			syncSend.wait();
			syncRecv.wait();
			break;
		}
	}
}
