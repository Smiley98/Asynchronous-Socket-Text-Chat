#pragma once
#include "../Network/Network.h"
#include <concurrent_vector.h>
#include <vector>

typedef concurrency::concurrent_vector<Packet> PacketBuffer;
class ClientBase :
	public Network
{
public:
	//Container of all packets received by this client from the server.
	PacketBuffer m_incoming;

	//Container of all packets waiting to be sent from this client to the server.
	PacketBuffer m_outgoing;

protected:
	//Tries to send all outgoing packets.
	void sendAll(int flags = 0, bool clear = true);

	//Blocks until there's no packets to receive.
	void recvAll(int flags = 0, bool add = true);

	//Tries to send the passed in packet to the server.
	bool send(const Packet& packet, int flags = 0);

	//Tries to receive packets from the server.
	bool recv(int flags = 0, bool add = true);

	//Tries to exchange a packet of the desired type with the server. 
	bool exchange(PacketType packetType);

	//Initialize Winsock2 and setup a client socket.
	void initialize();

	//Cleanup Winsock2, client socket, and client address.
	void shutdown();

	//Returns an array of indices corresponding to packets of type packetType within the specified packet buffer.
	std::vector<size_t> find(PacketType packetType, const PacketBuffer& packetBuffer);

	//Sidenotes:
	//1. I don't like how add() copies to out packets, but its allows us to buffer our output for a more elegant program so its necessary.
	//2. Implement an allocator that uses static memory. Vectors are resizing constantly which would be costly if scaled up.

private:
	ADDRINFO* m_address = NULL;
	SOCKET m_socket = INVALID_SOCKET;
};
