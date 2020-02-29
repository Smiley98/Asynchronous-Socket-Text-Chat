#pragma once
#include "../Network/Network.h"
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <atomic>

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
	Address m_fromAddress;//Sender
	//If we wanted to support multicasting, I guess we'd partition the packet such that the desired recipients are included via hole punching.
};
typedef std::vector<RoutedPacket> RoutedPacketBuffer;

struct ClientInfo {
	//Clients have 1000ms to ping the server otherwise they get disconnected.
	//double m_lastUpdated = 1000.0;
	std::atomic_bool m_active;
};

//Note: this server has no "receive" functionality. Its meant as a middle-man between clients, so all it does is re-route.
class ServerBase :
	public Network
{
protected:
	//Disconnect clients if they are inactive.
	void refresh();

	void handle(const Packet& packet);

	//Send till there's no more outgoing packets.
	void sendAll();
	//Don't be deceived. Despite passing in the sender's address, we're not guaranteed to send back to it.
	bool send(const Packet& packet, const Address& fromAddress);

	//These don't do anything special. There's no special behaviour on receive. Send however is a different story.
	void recvAll();
	bool recv();

//High level functions.
	//Send the packet to every client except for the passed in client (which is usually the original sender).
	bool reroute(const Packet& packet, const Address& exemptAddress);

	//Send packet to all clients.
	bool broadcast(const Packet& packet);

//Begin/end functions.
	//Initialize Winsock2 and setup a server socket.
	void initialize();

	//Cleanup Winsock2, server socket, and server address.
	void shutdown();

private:
	RoutedPacketBuffer m_incoming;
	RoutedPacketBuffer m_outgoing;
	//std::mutex m_incomingMutex;
	//std::mutex m_outgoingMutex;
	//Might not need mutexes.
	std::atomic_bool m_transfering;
	std::unordered_set<Address, AddressHash> m_clients;
	//I made this map because I don't wanna lift my Address structure into a Client structure (otherwise I need to modify my send/recv code).
	std::unordered_map<Address, ClientInfo, AddressHash> m_info;
	ADDRINFO* m_address = NULL;
	SOCKET m_socket = INVALID_SOCKET;
};
