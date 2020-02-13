#pragma once
#include <winsock2.h>
#include <concurrent_vector.h>
#include <array>
#include <atomic>
#include <cassert>
#include <string>
#define DELIMITER 69

size_t packetSize() {
	Packet<0> packet;
	return 0;
}

typedef unsigned char byte;
typedef std::atomic_uchar atomic_byte;

enum PacketType : byte {
	//Zero is reserved. If the packet type is zero, something is wrong!
	CONNECT = 1,
	QUIT,
	NONE,
	COUNT
};

//For optimal memory alignment, make count + sizeof(metadata) a multiple of 2.
template<size_t count>
class Packet :
	//count + 1 because I'm reserving space for a tailing null termination character to support string operations.
	public std::array<byte, count + 1>
{
public:
	Packet();
	Packet(const Packet<count>& packet);
	Packet& operator=(const Packet<count>& packet);
	std::string toString();
	void fromString(const std::string& string);
	void copyToPool(concurrency::concurrent_vector<Packet<count>>& packetPool);
	PacketType type;
private:
	void copyFrom(const Packet& packet);
	void autoFill();
};

class SmallPacket : public Packet<62> {};
class MediumPacket : public Packet<254> {};
class LargePacket : public Packet<1022> {};

typedef concurrency::concurrent_vector<SmallPacket> SmallPacketPool;
typedef concurrency::concurrent_vector<MediumPacket> MediumPacketPool;
typedef concurrency::concurrent_vector<LargePacket> LargePacketPool;

class Network
{
protected:
	void startupWSA();
	void cleanupWSA();
};

template<size_t count>
struct POD {
	//Writes data to packet.
	virtual void Serialize(Packet<count>& packet) = 0;

	//Reads data from packet.
	virtual void Deserialize(const Packet<count>& packet) = 0;
};

template<size_t count>
Packet<count>::Packet()
{
	autoFill();
	this->back() = PacketType::NONE;
}

template<size_t count>
Packet<count>::Packet(const Packet<count>& packet)
{
	copyFrom(packet);
}

template<size_t count>
Packet<count>& Packet<count>::operator=(const Packet<count>& packet)
{
	copyFrom(packet);
	return *this;
}

template<size_t count>
std::string Packet<count>::toString()
{
	if (this->front() == DELIMITER) return "";
	byte string[count];
	for (size_t i = 1; i < count; i++) {
		if (this->at(i) == DELIMITER) {
			memcpy(string, this->data(), i);
			string[i] = '\0';
			return std::string(reinterpret_cast<const char*>(string));
		}
	}
}

template<size_t count>
void Packet<count>::fromString(const std::string& string)
{
#if _DEBUG
	assert(string.size() < count);
#endif
	//string::copy() doesn't write '\0' to the end. Its essentially a memcpy for a c++ string.
	string.copy(reinterpret_cast<char* const>(this->data()), string.size());
}

template<size_t count>
void Packet<count>::copyToPool(concurrency::concurrent_vector<Packet<count>>& packetPool)
{
	packetPool.resize(packetPool.size() + 1);
	packetPool.back() = *this;
}

template<size_t count>
inline void Packet<count>::copyFrom(const Packet<count>& packet)
{
	memcpy(this->data(), const_cast<Packet&>(packet).data(), count);
}

template<size_t count>
inline void Packet<count>::autoFill()
{
	std::fill(this->begin(), this->end(), DELIMITER);
}
