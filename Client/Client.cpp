#include "Client.h"

bool Client::exchange(PacketType packetType)
{
	Packet packet(packetType);
	send(packet);
	//Peek rather than consume packets incase we've received a non-packetType packet.
	if (recv(MSG_PEEK))
		return find(packetType, m_incoming).size() > 0;
	return false;
}

void Client::sendAll(int flags, bool clear)
{
	for (const Packet& packet : m_outgoing)
		send(packet, flags);
	if(clear)
		m_outgoing.clear();
}

void Client::recvAll(int flags, bool add)
{
	while (recv(flags, add));
}

bool Client::send(const Packet& packet, int flags)
{
	return sendto(m_socket, packet.signedBytes(), packet.size(), flags, m_address->ai_addr, m_address->ai_addrlen) != SOCKET_ERROR;
}

bool Client::recv(int flags, bool add)
{
	Packet packet;
	if (recvfrom(m_socket, packet.signedBytes(), packet.size(), flags, NULL, NULL) != SOCKET_ERROR) {
		if (add)
			m_incoming.push_back(packet);
		return true;
	}
	return false;
}

std::vector<size_t> Client::find(PacketType packetType, const PacketBuffer& packetBuffer)
{
	std::vector<size_t> indices;
	for (size_t i = 0; i < packetBuffer.size(); i++) {
		if (packetBuffer[i].getType() == packetType)
			indices.push_back(i);
	}
	return indices;
}

void Client::initialize()
{
	Network::initialize();
	m_socket = createSocket();
	m_address = createAddress();
}

void Client::shutdown()
{
	closesocket(m_socket);
	freeaddrinfo(m_address);
	Network::shutdown();
}
