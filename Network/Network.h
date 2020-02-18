#pragma once
#include "Packet.h"
#include <WinSock2.h>
#include <WS2tcpip.h>

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
	static void initialize();
	static void shutdown();

	static SOCKET createSocket(bool blocking = false);
	static void destroySocket(SOCKET soc);

	static ADDRINFO* const createAddress(bool bind = false, const std::string& host = "localhost", const std::string& port = "6969");
	static void destroyAddress(ADDRINFO* const address);

	static void bindSocket(SOCKET soc, ADDRINFO* const address);
};
