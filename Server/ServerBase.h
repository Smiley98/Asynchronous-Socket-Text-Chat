#pragma once
#include "../Network/Network.h"
#include <unordered_map>

struct Address {
	Address();
	SOCKADDR_IN m_sai;
	int m_length;
	bool operator==(const Address& address) const;
};

struct AddressHash {
	ULONG operator()(const Address& key) const;
};

//I don't need multicasting.
//typedef std::vector<Address> AddressContainer;
//struct RoutedPacket {
//	Packet m_packet;
//	AddressContainer m_recipients;
//};

class ServerBase;
struct Client {
	PacketBuffer m_packets;
	Address m_address;
	bool operator==(const Client& client) const;
	bool sendTo(SOCKET soc, const Packet& packet) const;
	bool recvFrom(SOCKET soc, Packet& packet);
};

//Note: this server has no "receive" functionality. Its meant as a middle-man between clients, so all it does is re-route.
class ServerBase :
	public Network
{
protected:
//Highest level functions. They call the appropriate helper function based on the packet type and mode.
//These could probably go in the derived class.
	void send(const Packet& packet, const Client& sender);
	void recv();

//High level functions.
	//Tries to exchange a packet of the desired type with the desired client. 
	bool exchangeWith(PacketType packetType, Client& client);

	//Send the packet to every client except for the passed in client (which is usually the original sender).
	void reroute(const Packet& packet, const Client& exemptClient);

	//Send packet to all clients.
	void broadcast(const Packet& packet);
	
//Send functions.
	//Sends packet to the specified client.
	//void sendTo(const Packet& packet, const Client& client, int flags = 0);

	//Send all packets to the desired client.
	//void sendAllTo(const Client& client, int flags = 0);

//Receive functions.
	//Receives packet from the specified client.
	//bool recvFrom(Client& client);

	//Receive all packets from the specified client.
	//void recvAllFrom(Client& client);

	//Writes the contents of the received packet to the corresponding client, appends if the host data doesn't match an existing client.
	bool recvAny();

	//Receives anything and everything till we can't receive no more!
	//void recvAll();

//Begin/end functions.
	//Initialize Winsock2 and setup a server socket.
	void initialize();

	//Cleanup Winsock2, server socket, and server address.
	void shutdown();

private:
	std::unordered_map<Address, Client, AddressHash> m_clientMap;
	ADDRINFO* m_address = NULL;
	SOCKET m_socket = INVALID_SOCKET;

	void handlePacket(const Packet& packet);
};
