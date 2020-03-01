#include "ServerBase.h"
//We call recvfrom() here so we need to re-link Winsock2.
#pragma comment(lib, "Ws2_32.lib")
#include "../Common/Timer.h"
#include <cstdio>
#define TIMEOUT 2000.0

void ServerBase::process(Packet& packet)
{
	//Packet types turned out to be kind of useless.
	switch (packet.getType())
	{
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
		m_incoming.push_back({ packet, address });
		m_clients[address].m_active = true;
		if (packet.getType() == PacketType::STATUS_UPDATE)
			m_clients[address].m_status = (ClientStatus)packet.buffer()[0];
		//Do a switch here to examine buffer()[1]. Let's hope we can pass a number that corresponds to clients that should have their status' updated.
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
		//2. Broadcast a list of active clients (clients are responsible from removing themselves from this list).
		Packet packet(PacketType::LIST_ALL_ACTIVE, PacketMode::ONE_WAY);
		byte clientCount = m_clients.size();
		size_t offset = 1;
		packet.write(&clientCount, offset);
		for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++) {
			packet.write(&itr->first, sizeof(Address), offset);
			offset += sizeof(Address);
			//Prevent overflow, although we'd have to have a significant amount of clients for this to occur.
			if (offset + sizeof(Address) > Packet::bufferSize())
				break;
		}
		broadcast(packet);
		//Status update (I could put this in the above loop but I'm trying to minimize potential sources of error at this hour):
		Packet statusUpdate(PacketType::STATUS_UPDATE, PacketMode::ONE_WAY);
		size_t updateOffset = 0;
		for (auto itr = m_clients.begin(); itr != m_clients.end(); itr++) {
			statusUpdate.write(&itr->second.m_status, 1, updateOffset);
			updateOffset++;
		}
		broadcast(statusUpdate);
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
	//Read the number of addresses.
	const size_t addressCount = packet.buffer()[0];
	//+1 because we're reserving the first byte of the buffer for the address count.
	const size_t addressMemoryLength = 1 + addressCount * sizeof(Address);
	const size_t nonAddressMemoryLength = Packet::bufferSize() - addressMemoryLength;

	//Write the non-address memory of the original packet to the outgoing packet (including metadata).
	Packet outgoing;
	packet.read(outgoing.bytes(), nonAddressMemoryLength, addressMemoryLength);

	//Point to the start of the address information.
	const Address* address = reinterpret_cast<const Address*>(packet.buffer().data() + 1);

	bool result = true;
	//Despite address being const, we're changing its memory location so it holds a different value each iteration.
	for (size_t i = 0; i < addressCount; i++) {
		result &= address->sendTo(m_socket, outgoing);
		address += sizeof(Address);
	}
	return false;
	//And that my friends, is how we break our brain using pointer arithmetic and no dynamic allocation.
	//In a perfect world, I would derive from my NetworkObject class and override the serialize and deserialize methods.
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
