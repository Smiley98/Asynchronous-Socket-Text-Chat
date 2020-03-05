#include "ClientBase.h"
#include <iostream>
#include <mutex>
#define LOGGING false
#define LOCAL_HOST true

void ClientBase::copyIncoming(PacketBuffer& incoming, bool clear)
{
	m_incomingMutex.lock();
	incoming = m_incoming;
	if(clear)
		m_incoming.clear();
	m_incomingMutex.unlock();
}

void ClientBase::addOutgoing(const Packet& packet)
{
	m_outgoingMutex.lock();
	m_outgoing.push_back(packet);
	m_outgoingMutex.unlock();
}

bool ClientBase::exchange(PacketType packetType, PacketMode packetMode)
{
	Packet packet(packetType, packetMode);
	//Doesn't append to outgoing so no need to lock the outgoing mutex.
	send(packet);
	//Make sure the lock is released reguardless of the code path.
	std::lock_guard<std::mutex> guard(m_incomingMutex);
	if (recv())
		return findPacketOfType(packetType, m_incoming).size() > 0;
	return false;
}

void ClientBase::sendAll(int flags, bool clear)
{
	m_outgoingMutex.lock();
	for (const Packet& packet : m_outgoing)
		send(packet, flags);
	if(clear)
		m_outgoing.clear();
	m_outgoingMutex.unlock();
}

void ClientBase::recvAll(int flags, bool add)
{
	m_incomingMutex.lock();
	while (recv(flags, add));
	m_incomingMutex.unlock();
}

bool ClientBase::send(const Packet& packet, int flags)
{
	return sendto(m_socket, packet.signedBytes(), packet.size(), flags, m_address->ai_addr, m_address->ai_addrlen) != SOCKET_ERROR;
}

bool ClientBase::recv(int flags, bool add)
{
	Packet packet;
	if (recvfrom(m_socket, packet.signedBytes(), packet.size(), flags, NULL, NULL) != SOCKET_ERROR) {
		if (add)
			m_incoming.push_back(packet);
#if LOGGING
		printf("Client packet string: %s\n", packet.toString().c_str());
#endif
		return true;
	}
	return false;
}

void ClientBase::initialize()
{
	Network::initialize();
	m_socket = createSocket();
#if LOCAL_HOST
	m_address = createAddress();
#else
	printf("Please enter the host address.\n");
	std::string host;
	std::getline(std::cin, host);
	m_address = createAddress(false, host);
#endif
}

void ClientBase::shutdown()
{
	closesocket(m_socket);
	freeaddrinfo(m_address);
	Network::shutdown();
}
