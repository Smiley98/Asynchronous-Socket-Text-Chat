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

bool Client::operator==(const Client& client) const
{
	return m_address == client.m_address;
}

bool Client::sendTo(SOCKET soc, const Packet& packet) const
{
	return sendto(soc, packet.signedBytes(), packet.size(), 0, reinterpret_cast<const SOCKADDR*>(&m_address.m_sai), m_address.m_length) != SOCKET_ERROR;
}

bool Client::recvFrom(SOCKET soc, Packet& packet)
{
	Address address;
	if (recvfrom(soc, nullptr, 0, MSG_PEEK, reinterpret_cast<SOCKADDR*>(&address.m_sai), &address.m_length) != SOCKET_ERROR) {
		if(address == m_address)
			return recvfrom(soc, packet.signedBytes(), packet.size(), 0, reinterpret_cast<SOCKADDR*>(&m_address.m_sai), &m_address.m_length) != SOCKET_ERROR;
	}
	return false;
}
  
void ServerBase::send(const Packet& packet, const Client& sender)
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
	//case SPECIFIC:
	//	for (const Address& address : recipients) {
	//
	//	}
	//	break;
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

void ServerBase::recv()
{
}

//No good because we don't store packets.
//bool ServerBase::exchangeWith(PacketType packetType, Client& client)
//{
//	Packet packet(packetType, PacketMode::TWO_WAY);
//	client.sendTo(m_socket, packet);
//	if (client.recvFrom(m_socket, packet))
//		return true;//Figure out how to find later
//	//if (recvFrom(client))
//	//	return findPacketOfType(packetType, client.m_incoming).size() > 0;
//	return false;
//}

void ServerBase::broadcast(const Packet& packet)
{
	for (auto itr = m_clientMap.begin(); itr != m_clientMap.end(); itr++)
		itr->second.sendTo(m_socket, packet);
}

void ServerBase::reroute(const Packet& packet, const Client& exemptClient)
{
	for (auto itr = m_clientMap.begin(); itr != m_clientMap.end(); itr++) {
		if(itr->second == exemptClient)
			continue;
		itr->second.sendTo(m_socket, packet);
	}
}

//Moved to client.
//bool ServerBase::recvFrom(Client& client)
//{
//	Packet packet;
//	Address address;
//	//Peek the packet so we can compare the written address with that of the desired clients address (otherwise we'd overwrite the client's address which isn't good)!
//	if (recvfrom(m_socket, packet.signedBytes(), packet.size(), MSG_PEEK, reinterpret_cast<SOCKADDR*>(&address.m_sai), &address.m_length) != SOCKET_ERROR) {
//		if (address == client.m_address) {
//			//Consume the packet if its from the desired client.
//			client.recvFrom(m_socket, packet);
//			client.m_incoming.push_back(packet);
//			return true;
//		}
//	}
//	//Return false if nothing received in general or from the desired client.
//	return false;
//}

//void ServerBase::recvAllFrom(Client& client)
//{
//	while (recvFrom(client));
//}

bool ServerBase::recvAny()
{
	Packet packet;
	Address address;
	if (recvfrom(m_socket, packet.signedBytes(), packet.size(), 0, reinterpret_cast<SOCKADDR*>(&address.m_sai), &address.m_length) != SOCKET_ERROR) {
		//This is no good because ip strings aren't unique. Would be interesting to consider passing just the SAI cause that produced seemingly unique strings.
		//char ipBuffer[INET_ADDRSTRLEN];
		//inet_ntop(AF_INET, &address.m_sai.sin_addr, ipBuffer, sizeof(ipBuffer))
		
		return true;
	}
	return false;
}

//This is valid but I can't while this because there's currently no incoming/outgoing separation. Have to send upon receive.
//void ServerBase::recvAll()
//{
//	while (recvAny());
//}

//void ServerBase::sendTo(const Packet& packet, const Client& client, int flags)
//{
//	client.sendTo(m_socket, packet, flags);
//}

//I guess this would send all this client's outgoing packets?
//void ServerBase::sendAllTo(const Client& client, int flags)
//{
//	for (const Packet& packet : client.m_outgoing)
//		sendTo(packet, client, flags);
//}

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
