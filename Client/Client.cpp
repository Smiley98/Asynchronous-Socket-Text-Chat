#include "Client.h"
#include <future>
#include <cstdio>
#define CLIENT_LOGGING true

Client::Client()
{	//Leave it up to the user to start the client's networking thread.
	initialize();
}

Client::~Client()
{	//Make sure to stop if the user hasn't already!
	stop();
	shutdown();
}

void Client::start()
{
	if (!running()) {
		m_running = true;
		m_thread = std::thread(&Client::run, this);
	}
}

void Client::stop()
{
	if (running()) {
		m_running = false;
		m_thread.join();
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
	m_state.store(static_cast<const byte>(clientState));
#ifdef CLIENT_LOGGING
	switch (clientState)
	{
	case ClientState::IDLE:
		printf("Idling...\n");
		break;
	case ClientState::CONSUME:
		printf("Consuming.\n");
		break;
	default:
		break;
	}
#endif
}

void Client::run()
{
	while (running()) {
		switch (getState()) {
		case ClientState::IDLE:
			break;
		//Sends/receives all incoming/outgoing packets, leaving the client free to enque/deque packets at will!
		case ClientState::CONSUME:
			std::future<void> syncSend = std::async(&Client::sendAll, this, 0, true);
			std::future<void> syncRecv = std::async(&Client::recvAll, this, 0, true);
			syncSend.wait();
			syncRecv.wait();
			break;
		}
	}
}
