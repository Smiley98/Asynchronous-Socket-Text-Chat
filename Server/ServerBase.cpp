#include "ServerBase.h"
#include "../Common/Timer.h"
#include "../Common/NetworkObject.h"
#include <iostream>

//Quick timeout for debug purposes. Its not costly but might as well keep it between 1-2 seconds in production.
#define TIMEOUT 100.0
#define LOGGING true

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

		switch (packet.getType())
		{
			case PacketType::GET_THIS_CLIENT_INFORMATION: {
				ClientInformation clientInformation{ address, m_clients[address].m_status };
				Packet::serialize(clientInformation, packet);
				break;
			}
			
			case PacketType::SET_CLIENT_STATUS: {
				ClientInformation routedStatus;
				Packet::deserialize(packet, routedStatus);
				m_clients[routedStatus.m_address].m_status = routedStatus.m_status;
				break;
			}

			case PacketType::PLAYER: {
				Point data;
				Packet::deserialize(packet, data);
				printf("Server received: %s %h %h\n", packet.typeString().c_str(), data.x, data.y);
				break;
			}
			case PacketType::PUCK: {
				Puck data;
				Packet::deserialize(packet, data);
				printf("Server received: %s %h %h %h %h\n", packet.typeString().c_str(), data.position.x, data.position.y, data.velocity.x, data.velocity.y);
				break;
			}
			
			case PacketType::STRING:
	#if LOGGING
				printf("String packet: %s\n", packet.toString().c_str());
	#endif
				break;
			default:
				break;
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
	static Timer timer;
	if (timer.elapsed() >= TIMEOUT) {
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

bool ServerBase::multicast(const Packet& packet)
{
	//Extract addresses and information from packet, then send information to all addresses.
	std::vector<Address> addresses;
	Packet::deserialize(packet, addresses);

	const size_t dataStart = 1 + sizeof(Address) * addresses.size();
	const size_t dataSize = packet.buffer()[dataStart];

	//Yuck xD.
	Packet outgoing(static_cast<PacketType>(packet.buffer()[dataStart + 1]), static_cast<PacketMode>(packet.buffer()[dataStart + 2]));
	//Assume that we're writing a single object (deserialization expects the object count followed by the data).
	outgoing.buffer()[0] = 1;
	//Also yuck.
	packet.read(&outgoing.buffer()[1], dataSize - Packet::headerSize(), dataStart + Packet::headerSize() + 1);

	bool result = false;
	for (const Address& address : addresses)
		result &= address.sendTo(m_socket, outgoing);
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

bool ServerBase::broadcast(const Packet& packet)
{
	bool result = true;
	for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++)
		result &= itr->first.sendTo(m_socket, packet);
	return result;
}

void ServerBase::initialize()
{
	Network::initialize();
	m_socket = createSocket();
	printf("Please enter the host name.\n");
	//std::string host;
	//std::getline(std::cin, host);
	//m_address = createAddress(true, host, "8888");
	m_address = createAddress(true, "");
	bindSocket(m_socket, m_address);
}

void ServerBase::shutdown()
{
	destroySocket(m_socket);
	freeaddrinfo(m_address);
	Network::shutdown();
}
