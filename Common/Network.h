#pragma once
#include "Packet.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <utility>
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

//The server sorta already has this, but I'm running out of time xD
//Never mind about this structure.
//struct AddressPacket {
//	Packet m_packet;
//	std::vector<Address> m_addresses;
//};

struct Address {
	Address();
	//Construct from packet.
	//Address(const Packet& packet);
	SOCKADDR_IN m_sai;
	int m_length;
	bool operator==(const Address& address) const;

	bool sendTo(SOCKET soc, const Packet& packet) const;
	bool recvFrom(SOCKET soc, Packet& packet);

	void print() const;

	static std::vector<Address> decode(const Packet& packet);
	static Packet encode(const std::vector<Address>& addresses);
	
	//These would be more useful than what I have currently, but time is of the essence!
	//static Packet fuse(const std::vector<Address>& addresses, const Packet& packet);
	//static std::pair<std::vector<Address>, Packet> unfuse(const Packet& packet);
};

struct AddressHash {
	ULONG operator()(const Address& key) const;
};

//Its late
enum ClientStatus : byte {
	//NONE,//I don't have the time to figure out where there's an ambiguous NONE/COUNT xD
	A,
	FREE,
	IN_CHAT,
	IN_GAME,
	B
	//COUNT
};
