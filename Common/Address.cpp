#include "Address.h"

bool compareAddresses(const SOCKADDR_IN& a, const SOCKADDR_IN& b)
{
	return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
}

ULONG hashAddress(const Address& address)
{
	return address.m_sai.sin_addr.s_addr ^ address.m_sai.sin_port;
}

ULONG AddressHash::operator()(const Address& address) const
{
	return hashAddress(address);
}

Address::Address()
{
	memset(&m_sai, 0, sizeof(SOCKADDR_IN));
	m_length = sizeof(SOCKADDR_IN);
}

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

/*Packet Address::serialize(const std::vector<Address>& addresses, const std::vector<byte>& data)
{
	assert(1 + addresses.size() * sizeof(Address) + 1 + data.size() < Packet::size());
	Packet packet;

	//1. Write addresses.
	packet.buffer()[0] = addresses.size();
	size_t offset = 1;
	for (size_t i = 0; i < addresses.size(); i++) {
		packet.write(&addresses[i], sizeof(Address), offset);
		offset += sizeof(Address);
	}

	//2. Write data.
	const size_t dataSize = data.size();
	packet.write(&dataSize, 1, offset);
	packet.write(data.data(), data.size(), offset + 1);

	return packet;
}*/

/*std::pair<std::vector<Address>, std::vector<byte>> Address::deserialize(const Packet& packet)
{
	//1. Read addresses.
	const size_t addressCount = packet.buffer()[0];
	std::vector<Address> addresses(addressCount);
	const Address* address = reinterpret_cast<const Address*>(packet.buffer().data() + 1);
	for (size_t i = 0; i < addressCount; i++) {
		addresses[i] = *address;
		address++;
	}

	//2. Read data.
	const size_t dataStart = 1 + addresses.size() * sizeof(Address);
	std::vector<byte> data(packet.buffer()[dataStart]);
	packet.read(data.data(), data.size(), dataStart);

	return std::make_pair(addresses, data);
}*/

/*std::vector<Address> Address::decode(const Packet& packet)
{
	//assert(packet.getType() == PacketType::LIST_ALL_ACTIVE);	//Might not always be the case.
	//assert(packet.getMode() == PacketMode::MULTICAST);		//Also not always true cause of explicit operations.
	const size_t addressCount = packet.buffer()[0];
	std::vector<Address> result(addressCount);
	const Address* address = reinterpret_cast<const Address*>(packet.buffer().data() + 1);
	for (size_t i = 0; i < addressCount; i++) {
		result[i] = *address;
		address++;//I could be edgy and increment this counter in loop initialization, but that makes me stress.
	}
	return result;
}*/

/*Packet Address::encode(const std::vector<Address>& addresses)
{	//Type doesn't actually matter, will most likely be overwritten cause data should be added explicitly.
	Packet packet(PacketType::GENERIC, PacketMode::MULTICAST);
	packet.buffer()[0] = addresses.size();
	size_t offset = 1;
	//const Address& address : addresses//I'm scared of range based for loops cause idk if & will break whatever the iterator is implemented as.
	for (size_t i = 0; i < addresses.size(); i++) {
		packet.write(&addresses[i], sizeof(Address), offset);
		offset += sizeof(Address);
	}
	return packet;
}*/
