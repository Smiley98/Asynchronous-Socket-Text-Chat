#pragma once
#include "../Common/Network.h"
#include <unordered_map>

struct RoutedPacket {
	Packet m_packet;
	Address m_fromAddress;
};
typedef std::vector<RoutedPacket> RoutedPacketBuffer;

struct ClientInfo {
	bool m_active;
};

class ServerBase :
	public Network
{
protected:
	RoutedPacketBuffer m_incoming;
	RoutedPacketBuffer m_outgoing;

	//Parses the passed in packet, potentially writing to it.
	void process(Packet& packet);

	//Send till there's no more outgoing packets.
	void sendAll();

	//Don't be deceived. Despite passing in the sender's address, we're not guaranteed to send back to it.
	bool send(Packet& packet, const Address& fromAddress);

	//Receives till we can't receive no more!
	void recvAll();

	//Receive any packet that comes our way. Append its sender to our unique address list.
	bool recv();

//High level functions.
	//Disconnect clients if they are inactive.
	void refresh();

	//Reassigns outcoming to incoming and clears incoming.
	void transfer();

	//Send packet to all clients.
	bool broadcast(const Packet& packet);

	//Send the packet to every client except for the passed in client (which is usually the original sender).
	bool reroute(const Packet& packet, const Address& exemptAddress);

//Begin/end functions.
	//Initialize Winsock2 and setup a server socket.
	void initialize();

	//Cleanup Winsock2, server socket, and server address.
	void shutdown();

private:
	//Would be faster as an unordered_set, but I would have to rewrite stuff (ClientInfo would own an Address).
	std::unordered_map<Address, ClientInfo, AddressHash> m_clients;

	ADDRINFO* m_address = NULL;
	SOCKET m_socket = INVALID_SOCKET;
};
