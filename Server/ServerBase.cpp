#include "ServerBase.h"
#include "../Common/Timer.h"
#include "../Common/NetworkObjects.h"
#include "../Common/Multicast.h"
#include <iostream>
#define REFRESH_FREQUENCY 1000.0
#define LOGGING true

size_t ServerBase::s_id = 0;

void ServerBase::sendAll()
{
	for (const RoutedPacket& routedPacket : m_outgoing)
		send(const_cast<Packet&>(routedPacket.m_packet), routedPacket.m_fromAddress);
}

bool ServerBase::send(Packet& packet, const Address& fromAddress)
{
	bool result = true;
	switch (packet.getMode())
	{
	case PacketMode::TWO_WAY: {
		//Deserialized address matches from address, deserialized id matches from id... Why isn't this making it to the client correctly?
		//ClientInformation ci;
		//Packet::deserialize(packet, ci);
		//ci.m_address.print();
		//fromAddress.print();
		//printf("Id: %zu.\n", m_clients[fromAddress].m_id);
		//printf("Id: %zu.\n", ci.m_id);
		result = fromAddress.sendTo(m_socket, packet);
	}
	case PacketMode::REROUTE:
		result = reroute(packet, fromAddress);
		break;
	case PacketMode::BROADCAST:
		result = broadcast(packet);
		break;
	case PacketMode::MULTICAST:
		result = multicast(packet);
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
		m_clients[address].m_active = true;
		//0 means new client so assign it a unique id!
		if (m_clients[address].m_id == 0)
			m_clients[address].m_id = ++s_id;

		static Address initialAddress;
		if (m_clients.size() == 1) {
			printf("-----ONE ADDRESS------\n");
			initialAddress = m_clients.begin()->first;
			initialAddress.print();
			printf("--------------------\n\n");
		}

		else if (m_clients.size() >= 2) {
			printf("-----TWO ADDRESSES-----\n");
			for (auto i : m_clients) {
				printf("Hash: %llu.\n", hashAddress(i.first));
				i.first.print();
			}
			auto itr = m_clients.begin();
			if (itr->first == (++itr)->first)
				printf("1st and 2nd are equal, not good...\n");
			else
				printf("1st and 2nd aren't equal, good!\n");
			printf("---------------------\n\n");

			for (auto i : m_clients) {
				if (i.first == initialAddress) {
					if (i.second.m_id == 1)
						printf("Yay the initial address key has an id of 1!\n");
					break;
				}
			}
		}
		
		//static Address initialAddress;
		//if (m_clients.size() == 1) {
		//	printf("***ONE CLIENT***\n");
		//	printf("Initial client id is %zu.\n", m_clients[address].m_id);
		//	address.print();
		//	printf("****************\n\n");
		//	initialAddress = address;
		//}
		//
		//if (m_clients.size() >= 2) {
		//	auto i1 = m_clients.begin();
		//	auto i2 = m_clients.begin()++;
		//	printf("***TWO CLIENTS***\n");
		//	printf("Client 1 id: %zu.\nClient 2 id: %zu.\n", i1->second.m_id, i2->second.m_id);
		//	printf("----Client 1 address----\n");
		//	i1->first.print();
		//	printf("----Client 2 address----\n");
		//	i2->first.print();
		//	if (initialAddress == i1->first)
		//		printf("Initial address is equal to current beginning address.\n\n");
		//	printf("*****************\n\n");
		//}

		switch (packet.getType())
		{
			case PacketType::GET_THIS_CLIENT_INFORMATION: {
				ClientInformation clientInformation{ address, m_clients[address].m_id, m_clients[address].m_status };
				Packet::serialize(clientInformation, packet);
				break;
			}

			case PacketType::SET_CLIENT_STATUS: {
				ClientInformation routedStatus;
				Packet::deserialize(packet, routedStatus);
				m_clients[routedStatus.m_address].m_status = routedStatus.m_status;
				break;
			}
		}
#if LOGGING
		if(!packet.typeString().empty())
			printf("Server received packet of type %s from client %zu.\n", packet.typeString().c_str(), m_clients[address].m_id);
#endif

		//No need to append the packet if its not meant to be routed.
		if (packet.getMode() != PacketMode::ONE_WAY)
			m_incoming.push_back({ packet, address });

		return true;
	}
	return false;
}

void ServerBase::refresh()
{
	static Timer timer;
	if (timer.elapsed() >= REFRESH_FREQUENCY) {
		timer.restart();

		//1. Disconnect any inactive clients.
		auto itr = m_clients.begin();
		while (itr != m_clients.end()) {
			if (itr->second.m_active) {
				itr->second.m_active = false;
				itr++;
			}
			else
				itr = m_clients.erase(itr);
		}

		//2. Broadcast client information.
		std::vector<ClientInformation> allClientInformation(m_clients.size());
		size_t count = 0;
		for (const auto& i : m_clients) {
			allClientInformation[count].m_address = i.first;
			allClientInformation[count].m_status = i.second.m_status;
			allClientInformation[count].m_id = i.second.m_id;
			count++;
		}

		Packet packet(PacketType::GET_ALL_CLIENT_INFORMATION, PacketMode::ONE_WAY);
		Packet::serialize(allClientInformation, packet);
		for (const ClientInformation& clientInformation : allClientInformation) {
			clientInformation.m_address.sendTo(m_socket, packet);
		}
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

bool ServerBase::multicast(const Packet& packet)
{
	bool result = true;
	MulticastPacket multicastPacket = Multicast::deserialize(packet);
	for (const Address& address : multicastPacket.m_addresses)
		result &= address.sendTo(m_socket, multicastPacket.m_packet);
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
