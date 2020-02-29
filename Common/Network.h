#pragma once
#include "Packet.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
typedef std::vector<Packet> PacketBuffer;

//Returns the indices in which packets of type packetType can be found within the packet buffer.
std::vector<size_t> findPacketOfType(PacketType packetType, const PacketBuffer& packetBuffer);

//Writes object information to a packet.
class Serializer {
public:
	virtual void serialize(Packet& packet) = 0;
};

//Forms an object (this) from a packet.
class Deserializer {
public:
	virtual void deserialize(const Packet& packet) = 0;
};

//Network object interface.
class NetworkObject
	: public Serializer, public Deserializer
{
public:
	virtual void serialize(Packet& packet) = 0;
	virtual void deserialize(const Packet& packet) = 0;
};
  
class Network abstract {
public:
//protected:
	static void initialize();
	static void shutdown();

	static SOCKET createSocket(bool blocking = false);
	static void destroySocket(SOCKET soc);

	static ADDRINFO* const createAddress(bool bind = false, const std::string& host = "localhost", const std::string& port = "6969");
	static void destroyAddress(ADDRINFO* const address);

	static void bindSocket(SOCKET soc, ADDRINFO* address);
};

struct AddressHash {
	ULONG operator()(const Address& key) const;
};

struct Address {
	Address();
	SOCKADDR_IN m_sai;
	int m_length;
	bool operator==(const Address& address) const;

	bool sendTo(SOCKET soc, const Packet& packet) const;
	bool recvFrom(SOCKET soc, Packet& packet);
};
