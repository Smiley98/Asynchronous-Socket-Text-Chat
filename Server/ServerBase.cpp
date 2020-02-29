#include "ServerBase.h"
#include <iostream>
#include <thread>
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

void ServerBase::sendAll()
{
}

bool ServerBase::send(const Packet& packet, const Address& address)
{
	bool result = false;
	switch (packet.getMode())
	{
	case REROUTE:
		reroute(packet, address);
		break;
	case BROADCAST:
		broadcast(packet);
		break;
	case ERROR:
		printf("Error packet received!\n");
		printf("Error packet contents: %s", packet.toString().c_str());
		break;
	default:
		result = address.sendTo(m_socket, packet);
		break;
	}
	return result;
}

bool ServerBase::broadcast(const Packet& packet)
{
	//if (!m_transfering.load())
	//	return;
	bool result = true;
	for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++)
		result &= itr->sendTo(m_socket, packet);
	return result;
}

bool ServerBase::reroute(const Packet& packet, const Address& exemptClient)
{
	//if (!m_transfering.load())
	//	return;
	bool result = true;
	for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++) {
		if(*itr == exemptClient)
			continue;
		result &= itr->sendTo(m_socket, packet);
	}
	return result;
}

void ServerBase::recvAll()
{
	while (recv());
}

bool ServerBase::recv()
{
	//if (!m_transfering.load())
	//	return false;
	Packet packet;
	Address address;
	if (recvfrom(m_socket, packet.signedBytes(), packet.size(), 0, reinterpret_cast<SOCKADDR*>(&address.m_sai), &address.m_length) != SOCKET_ERROR) {
		m_clients.insert(address);
		m_incoming.push_back({ packet, address });
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
