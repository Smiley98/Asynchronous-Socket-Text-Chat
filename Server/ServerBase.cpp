#include "ServerBase.h"
#include "../Common/Timer.h"
#include "../Common/NetworkObjects.h"
#include "../Common/Multicast.h"
#include <iostream>
#define DISCONNET_FREQUENCY 5.0
#define UPDATE_FREQUENCY 0.1
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
	case PacketMode::TWO_WAY:
		result = fromAddress.sendTo(m_socket, packet);
		break;
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

		//Thing to use in logs.
		Point data;
#if LOGGING
		if (!packet.typeString().empty())
			printf("Server received packet of type %s from client %zu.\n", packet.typeString().c_str(), m_clients[address].m_id);
#endif
		switch (packet.getType())
		{
			case PacketType::THIS_CLIENT_INFORMATION: {
				ClientInformation clientInformation{ address, m_clients[address].m_id,  m_clients[address].m_status };
				Packet::serialize(clientInformation, packet);
				break;
			}

			case PacketType::SET_CLIENT_STATUS: {
				ClientInformation status;
				Packet::deserialize(packet, status);
				m_clients[status.m_address].m_status = status.m_status;
				break;
			}
#if LOGGING
			case PacketType::KINEMATIC: {
				Kinematic kinematic;
				Packet::deserialize(packet, kinematic);
				//No need to output the z component, but you can if you want.
				printf("Position: %f %f.\n", kinematic.position.x, kinematic.position.y/*, kinematic.position.z*/);
				printf("Velocity: %f %f.\n", kinematic.velocity.x, kinematic.velocity.y/*, kinematic.velocity.z*/);
				printf("Acceleration: %f %f.\n", kinematic.acceleration.x, kinematic.acceleration.y/*, kinematic.acceleration.z*/);
				break;
			}
#endif
		}

		//No need to append the packet if its not meant to be routed.
		if (packet.getMode() != PacketMode::ONE_WAY)
			m_incoming.push_back({ packet, address });

		return true;
	}
	return false;
}

void ServerBase::refresh()
{
	static Timer disconnectTimer;
	if (disconnectTimer.elapsed() >= DISCONNET_FREQUENCY) {
		disconnectTimer.restart();

		auto itr = m_clients.begin();
		while (itr != m_clients.end()) {
			if (itr->second.m_active) {
				itr->second.m_active = false;
				itr++;
			}
			else
				itr = m_clients.erase(itr);
		}
	}

	static Timer updateTimer;
	if (updateTimer.elapsed() >= UPDATE_FREQUENCY) {
		updateTimer.restart();

		std::vector<ClientInformation> allClientInformation(m_clients.size());
		size_t count = 0;
		for (const auto& i : m_clients) {
			allClientInformation[count].m_address = i.first;
			allClientInformation[count].m_status = i.second.m_status;
			allClientInformation[count].m_id = i.second.m_id;
			count++;
		}

		Packet allClientsPacket(PacketType::ALL_CLIENT_INFORMATION, PacketMode::ONE_WAY);
		Packet::serialize(allClientInformation, allClientsPacket);
		for (const ClientInformation& clientInformation : allClientInformation)
			clientInformation.m_address.sendTo(m_socket, allClientsPacket);
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
