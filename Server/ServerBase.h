#pragma once
#include "../Network/Network.h"
#include <concurrent_vector.h>
#include <unordered_set>

struct Address {
	Address();
	SOCKADDR_IN m_sai;
	int m_length;
	bool operator==(const Address& address) const;

	bool sendTo(SOCKET soc, const Packet& packet) const;
	bool recvFrom(SOCKET soc, Packet& packet);
};

struct AddressHash {
	ULONG operator()(const Address& key) const;
};

struct RoutedPacket {
	Packet m_packet;
	Address m_address;
};

//Note: this server has no "receive" functionality. Its meant as a middle-man between clients, so all it does is re-route.
class ServerBase :
	public Network
{
protected:
	//
	void loop();

	//
	void send(const Packet& packet, const Address& sender);

	//Writes the contents of the received packet to the corresponding client, appends if the host data doesn't match an existing client.
	bool recv();

//High level functions.
	//Send the packet to every client except for the passed in client (which is usually the original sender).
	void reroute(const Packet& packet, const Address& exemptAddress);

	//Send packet to all clients.
	void broadcast(const Packet& packet);

//Begin/end functions.
	//Initialize Winsock2 and setup a server socket.
	void initialize();

	//Cleanup Winsock2, server socket, and server address.
	void shutdown();

private:
	//No differentiation between incoming and outgoing because recevied packets are immediately sent rather than explicitly forming an outgoing buffer.
	//We can actually accelerate this by constantly receiving and swapping at regular intervals.
	concurrency::concurrent_vector<RoutedPacket> m_packets;
	std::unordered_set<Address, AddressHash> m_clients;
	ADDRINFO* m_address = NULL;
	SOCKET m_socket = INVALID_SOCKET;

	void handlePacket(const Packet& packet);
};
