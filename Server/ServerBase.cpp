#include "ServerBase.h"
//We call recvfrom() here so we need to re-link Winsock2.
#pragma comment(lib, "Ws2_32.lib")
#include "../Common/Timer.h"
#include <cstdio>
#define TIMEOUT 5000.0
#define LOGGING true

void ServerBase::process(Packet& packet)
{
	switch (packet.getType())
	{
	case PacketType::GENERIC:
		break;
	case PacketType::CONNECT:
		break;
	case PacketType::DISCONNECT:
		break;
	case PacketType::LIST_ALL_ACTIVE: {
		size_t offset = 0;
		for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++) {
			packet.write(&itr->first, sizeof(Address), offset);
			offset += sizeof(Address);
		}
	}
	case PacketType::STRING:
		printf("String packet: %s\n", packet.toString().c_str());
		break;
	default:
		break;
	}
}

void ServerBase::sendAll()
{
	for (const RoutedPacket& routedPacket : m_outgoing)
		send(const_cast<Packet&>(routedPacket.m_packet), routedPacket.m_fromAddress);
}

bool ServerBase::send(Packet& packet, const Address& fromAddress)
{
	process(packet);
	bool result = false;
	switch (packet.getMode())
	{
	case PacketMode::ONE_WAY:
		break;
	case PacketMode::TWO_WAY:
		result = fromAddress.sendTo(m_socket, packet);
	case PacketMode::REROUTE:
		result = reroute(packet, fromAddress);
		break;
	case PacketMode::BROADCAST:
		result = broadcast(packet);
		break;
	default:
		break;
	}
	return result;
}

void ServerBase::recvAll()
{
	while (recv());
}

bool ServerBase::recv()
{
	Packet packet;
	Address address;
	if (recvfrom(m_socket, packet.signedBytes(), packet.size(), 0, reinterpret_cast<SOCKADDR*>(&address.m_sai), &address.m_length) != SOCKET_ERROR) {
		m_incoming.push_back({ packet, address });
		m_clients[address].m_active = true;
#if LOGGING
		printf("Server packet string: %s\n", packet.toString().c_str());
#endif
		return true;
	}
	return false;
}

void ServerBase::refresh()
{
	static Timer timer;
	if (timer.elapsed() >= TIMEOUT) {
		for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++) {
			if (itr->second.m_active)
				itr->second.m_active = false;
			else
				itr = m_clients.erase(itr);
		}
		timer.restart();
	}
}

void ServerBase::transfer()
{
	m_outgoing = m_incoming;
	m_incoming.clear();
}

bool ServerBase::broadcast(const Packet& packet)
{
	bool result = true;
	for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++)
		result &= itr->first.sendTo(m_socket, packet);
	return result;
}

bool ServerBase::reroute(const Packet& packet, const Address& exemptClient)
{
	bool result = true;
	for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++) {
		if (itr->first == exemptClient)
			continue;
		result &= itr->first.sendTo(m_socket, packet);
	}
	return result;
}

void ServerBase::initialize()
{
	Network::initialize();
	m_socket = createSocket();
	m_address = createAddress(true, "");
	bindSocket(m_socket, m_address);
}

void ServerBase::shutdown()
{
	destroySocket(m_socket);
	freeaddrinfo(m_address);
	Network::shutdown();
}
