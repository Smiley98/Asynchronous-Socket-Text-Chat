#include "ClientBase.h"

bool ClientBase::exchange(PacketType packetType)
{
	Packet packet(packetType);
	send(packet);
	if (recv())
		return findPacketOfType(packetType, m_incoming).size() > 0;
	return false;
}

void ClientBase::sendAll(int flags, bool clear)
{
	for (const Packet& packet : m_outgoing)
		send(packet, flags);
	if(clear)
		m_outgoing.clear();
}

void ClientBase::recvAll(int flags, bool add)
{
	while (recv(flags, add));
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
		return true;
	}
	return false;
}

void ClientBase::initialize()
{
	Network::initialize();
	m_socket = createSocket();
	m_address = createAddress();
}

void ClientBase::shutdown()
{
	closesocket(m_socket);
	freeaddrinfo(m_address);
	Network::shutdown();
}
