#pragma once
#include "../Common/Network.h"
#include "../Common/ClientInfo.h"
#include <unordered_map>

struct RoutedPacket {
	Packet m_packet;
	Address m_fromAddress;
};
typedef std::vector<RoutedPacket> RoutedPacketBuffer;

class ServerBase :
	public Network
{
protected:
	RoutedPacketBuffer m_incoming;
	RoutedPacketBuffer m_outgoing;

	//Send till there's no more outgoing packets.
	void sendAll();

	//Don't be deceived. Despite passing in the sender's address, we're not guaranteed to send back to it.
	bool send(Packet& packet, const Address& fromAddress);

	//Receives till we can't receive no more!
	void recvAll();

	//Receive any packet that comes our way. Append its sender to our unique address list.
	bool recv();

	//Disconnect clients if they are inactive.
	void refresh();

	//Reassigns outcoming to incoming and clears incoming.
	void transfer();

	//Sends the packet to all specified clients (based on the packet internals).
	bool multicast(const Packet& packet);

	//Send the packet to every client except for the passed in client (which is usually the original sender).
	bool reroute(const Packet& packet, const Address& exemptAddress);

	//Send the packet to all clients.
	bool broadcast(const Packet& packet);

	//Initialize Winsock2 and setup a server socket.
	void initialize();

	//Cleanup Winsock2, server socket, and server address.
	void shutdown();

private:
	std::unordered_map<Address, ClientDesc, AddressHash> m_clients;
	//std::unordered_map<ULONG, Address> m_hash;//Mapping of address hash values to addresses. Could remove the need to send entire addresses.
	ADDRINFO* m_address = NULL;
	SOCKET m_socket = INVALID_SOCKET;
};
