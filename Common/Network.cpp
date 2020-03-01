#include "Network.h"
#pragma comment(lib, "Ws2_32.lib")
#include <cstdio>

void Network::initialize()
{
	WSADATA wsa;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (result != 0) {
		printf("Winsock2 initialization failed (%i).\n", result);
		shutdown();
	}
}

void Network::shutdown()
{
	WSACleanup();
}

SOCKET Network::createSocket(bool blocking)
{
	SOCKET soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (soc == INVALID_SOCKET) {
		printf("Socket creation failed (%i).\n", WSAGetLastError());
		shutdown();
	}

	u_long mode = blocking ? 0 : 1;
	if (ioctlsocket(soc, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("Socket I/O control failed (%i).\n", WSAGetLastError());
		destroySocket(soc);
		shutdown();
	}

	return soc;
}

void Network::destroySocket(SOCKET soc)
{
	if (closesocket(soc) == INVALID_SOCKET) {
		printf("Socket close failed (%i).\n", WSAGetLastError());
		shutdown();
	}
}

ADDRINFO* const Network::createAddress(bool bind, const std::string& host, const std::string& port)
{
	ADDRINFO* addressInfo = NULL, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = bind ? AI_PASSIVE : 0;
	const char* nodeName = host.empty() ? NULL : host.c_str();
	const char* serviceName = port.empty() ? NULL : port.c_str();
	if (getaddrinfo(nodeName, serviceName, &hints, &addressInfo) != 0) {
		printf("Address resolution failed (%i).\n", WSAGetLastError());
		shutdown();
	}
	return addressInfo;
}

void Network::destroyAddress(ADDRINFO* const address)
{
	freeaddrinfo(address);
}

void Network::bindSocket(SOCKET soc, ADDRINFO* address)
{
	if (bind(soc, address->ai_addr, static_cast<int>(address->ai_addrlen)) == SOCKET_ERROR) {
		printf("Bind failed (%i).\n", WSAGetLastError());
		destroyAddress(address);
		destroySocket(soc);
		shutdown();
	}
}

std::vector<size_t> findPacketOfType(PacketType packetType, const PacketBuffer& packetBuffer)
{
	std::vector<size_t> indices;
	for (size_t i = 0; i < packetBuffer.size(); i++) {
		if (packetBuffer[i].getType() == packetType)
			indices.push_back(i);
	}
	return indices;
}

bool compareAddresses(const SOCKADDR_IN& a, const SOCKADDR_IN& b)
{
	return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
}

ULONG AddressHash::operator()(const Address& key) const
{
	return key.m_sai.sin_addr.s_addr ^ key.m_sai.sin_port;
}

Address::Address()
{
	memset(&m_sai, 0, sizeof(SOCKADDR_IN));
	m_length = sizeof(SOCKADDR_IN);
}

//Address::Address(const Packet& packet)
//{
//	packet.read(this, sizeof(Address));
//}

bool Address::operator==(const Address& address) const
{
	return compareAddresses(m_sai, address.m_sai);
}

bool Address::sendTo(SOCKET soc, const Packet& packet) const
{
	return sendto(soc, packet.signedBytes(), packet.size(), 0, reinterpret_cast<const SOCKADDR*>(&m_sai), m_length) != SOCKET_ERROR;
}

bool Address::recvFrom(SOCKET soc, Packet& packet)
{
	Address address;
	if (recvfrom(soc, nullptr, 0, MSG_PEEK, reinterpret_cast<SOCKADDR*>(&address.m_sai), &address.m_length) != SOCKET_ERROR) {
		if (address == *this)
			return recvfrom(soc, packet.signedBytes(), packet.size(), 0, reinterpret_cast<SOCKADDR*>(&m_sai), &m_length) != SOCKET_ERROR;
	}
	return false;
}

void Address::print() const
{
	printf("Ip address: %lu, port %hu.\n", m_sai.sin_addr.s_addr, m_sai.sin_port);
}

std::vector<Address> Address::deserialize(const Packet& packet)
{
	assert(packet.getType() == PacketType::LIST_ALL_ACTIVE);
	const size_t addressCount = packet.buffer()[0];
	std::vector<Address> result(addressCount);
	const Address* address = reinterpret_cast<const Address*>(packet.buffer().data() + 1);
	for (size_t i = 0; i < addressCount; i++) {
		result[i] = *address;
		address++;//I could be edgy and increment this counter in loop initialization, but that makes me stress.
	}
	return result;
}
