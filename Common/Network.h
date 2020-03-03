#pragma once
#include "Packet.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
typedef std::vector<Packet> PacketBuffer;
typedef std::vector<byte> ByteVector;

//Returns the indices in which packets of type packetType can be found within the packet buffer.
std::vector<size_t> findPacketOfType(PacketType packetType, const PacketBuffer& packetBuffer);
  
class Network abstract {
protected:
	static void initialize();
	static void shutdown();

	static SOCKET createSocket(bool blocking = false);
	static void destroySocket(SOCKET soc);

	static ADDRINFO* const createAddress(bool bind = false, const std::string& host = "localhost", const std::string& port = "6969");
	static void destroyAddress(ADDRINFO* const address);

	static void bindSocket(SOCKET soc, ADDRINFO* address);
};
