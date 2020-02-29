#include "ServerBase.h"
#include <iostream>
#include <thread>
#include "../Network/Timer.h"
#undef ERROR //Lol something in windows made this 0 and it messes up my enums xD
#define TIMEOUT 5000.0

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

void ServerBase::refresh()
{
	static Timer timer;
	if (fmod(timer.elapsed(), TIMEOUT) == 0.0) {
		for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++) {
			ClientInfo& clientInfo = m_info[*itr];
			if (clientInfo.m_active.load())
				clientInfo.m_active.store(false);
			else {
				itr = m_clients.erase(itr);
				m_info.erase(*itr);
			}
		}
		timer.restart();
	}
}

void ServerBase::handle(const Packet& packet)
{
	switch (packet.getType())
	{
	case GENERIC:
		break;
	case CONNECT:
		break;
	case DISCONNECT:
		break;
	case LIST_ALL_ACTIVE:

	case ERROR:
		break;
	default:
		break;
	}
}

void ServerBase::sendAll()
{
	for (const RoutedPacket& routedPacket : m_outgoing)
		send(routedPacket.m_packet, routedPacket.m_fromAddress);
}

bool ServerBase::send(const Packet& packet, const Address& fromAddress)
{
	bool result = false;
	switch (packet.getMode())
	{
	case ONE_WAY:
		break;
	case TWO_WAY:
		result = fromAddress.sendTo(m_socket, packet);
	case REROUTE:
		result = reroute(packet, fromAddress);
		break;
	case BROADCAST:
		result = broadcast(packet);
		break;
	case ERROR:
		printf("Error packet received!\n");
		printf("Error packet contents: %s", packet.toString().c_str());
		break;
	default:
		break;
	}
	//Handle the packet and return the result no matter what.
	//PacketMode decides how to send pack the received packet. PacketType decides if we switch states.
	handle(packet);
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
		m_info[address].m_active.store(true);
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
