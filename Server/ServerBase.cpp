#include "ServerBase.h"
#include <iostream>
#undef ERROR //Lol something in windows made this 0 and it messes up my enums xD

bool compareAddresses(const SOCKADDR_IN& a, const SOCKADDR_IN& b)
{
	return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
}

ULONG AddressHash::operator()(const Address& key) const
{
	return key.m_sai.sin_addr.s_addr ^ key.m_sai.sin_port;
}

Address::Address()
{
	memset(this, 0, sizeof(Address));
}

bool Address::operator==(const Address& address) const
{
	return compareAddresses(m_sai, address.m_sai);
}

bool Address::sendTo(SOCKET soc, const Packet& packet) const
{
	return sendto(soc, packet.signedBytes(), packet.size(), 0, reinterpret_cast<const SOCKADDR*>(&m_sai), m_length) != SOCKET_ERROR;
}

bool Address::recvFrom(SOCKET soc, Packet& packet)
{
	Address address;
	if (recvfrom(soc, nullptr, 0, MSG_PEEK, reinterpret_cast<SOCKADDR*>(&address.m_sai), &address.m_length) != SOCKET_ERROR) {
		if(address == *this)
			return recvfrom(soc, packet.signedBytes(), packet.size(), 0, reinterpret_cast<SOCKADDR*>(&m_sai), &m_length) != SOCKET_ERROR;
	}
	return false;
}

void ServerBase::loop()
{
}

void ServerBase::send(const Packet& packet, const Address& sender)
{
	switch (packet.getMode())
	{
	case ONE_WAY:
		handlePacket(packet);
		break;
	case TWO_WAY:
		sender.sendTo(m_socket, packet);
		handlePacket(packet);
		break;
	case REROUTE:
		reroute(packet, sender);
		handlePacket(packet);
		break;
	case BROADCAST:
		broadcast(packet);
		handlePacket(packet);
		break;
	case ERROR:
		printf("Error packet received!\n");
		printf("Error packet contents: %s", packet.toString().c_str());
		break;
	default:
		break;
	}
}

void ServerBase::handlePacket(const Packet& packet)
{
	switch (packet.getType())
	{
	case GENERIC:
		break;
	case CONNECT:
		break;
	case DISCONNECT:
		break;
	case ERROR:
		break;
	default:
		break;
	}
}

void ServerBase::broadcast(const Packet& packet)
{
	for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++)
		itr->sendTo(m_socket, packet);
}

void ServerBase::reroute(const Packet& packet, const Address& exemptClient)
{
	for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++) {
		if(*itr == exemptClient)
			continue;
		itr->sendTo(m_socket, packet);
	}
}

bool ServerBase::recv()
{
	Packet packet;
	Address address;
	if (recvfrom(m_socket, packet.signedBytes(), packet.size(), 0, reinterpret_cast<SOCKADDR*>(&address.m_sai), &address.m_length) != SOCKET_ERROR) {
		m_clients.insert(address);
		m_packets.push_back({ packet, address });
		return true;
	}
	return false;
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
