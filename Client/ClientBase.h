#pragma once
#include "../Common/Network.h"
#include <mutex>

class ClientBase :
	public Network
{
public:
	void copyIncoming(PacketBuffer& incoming, bool clear = true);
	void addOutgoing(const Packet& packet);

protected:
	//Container of all packets received by this client from the server.
	PacketBuffer m_incoming;

	//Container of all packets waiting to be sent from this client to the server.
	PacketBuffer m_outgoing;

	//Tries to exchange a packet of the desired type with the server. 
	bool exchange(PacketType packetType, PacketMode packetMode);

	//Tries to send all outgoing packets.
	void sendAll(int flags = 0, bool clear = true);

	//Blocks until there's no packets to receive.
	void recvAll(int flags = 0, bool add = true);

	//Tries to send the passed in packet to the server. (Not thread safe).
	bool send(const Packet& packet, int flags = 0);

	//Tries to receive packets from the server. (Not thread safe).
	bool recv(int flags = 0, bool add = true);

	//Initialize Winsock2 and setup a client socket.
	void initialize();

	//Cleanup Winsock2, client socket, and client address.
	void shutdown();

	//TODO: Make a static allocator for the packet buffers so vectors can grow/shrink faster!
private:
	ADDRINFO* m_address = NULL;
	SOCKET m_socket = INVALID_SOCKET;
	std::mutex m_incomingMutex;
	std::mutex m_outgoingMutex;
};
