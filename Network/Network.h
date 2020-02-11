#pragma once
#include <winsock2.h>
#include <concurrent_vector.h>
#include <array>
#include <atomic>
#include <cassert>
#include <string>
#define DELIMITER 69

typedef unsigned char byte;
typedef std::atomic_uchar atomic_byte;

enum PacketType : byte {
	//Zero is reserved. If the packet type is zero, something is wrong!
	CONNECT = 1,
	QUIT,
	NONE,
	COUNT
};

//Not the worst idea, but implementing this would mess up either back() or front(), and actually complicate to/from string. 
/*struct Metadata {
	union {
		struct {
			byte metadata[8] = { '\0' };
		};
		struct {
			byte null;
			byte type;
			byte reserved_byte_3;
			byte reserved_byte_4;
			byte reserved_byte_5;
			byte reserved_byte_6;
			byte reserved_byte_7;
			byte reserved_byte_8;
		};
	};
};*/

template<size_t count>
class Packet :
	public std::array<byte, count>
{
public:
	Packet();
	//Packet(const char* string);
	//Packet(const std::string& string);
	Packet(const Packet<count>& packet);
	Packet& operator=(const Packet<count>& packet);
	std::string toString();
	void fromString(const std::string& string);
	PacketType getType();
	void setType(PacketType packetType);
	void copyToPool(concurrency::concurrent_vector<Packet<count>>& packetPool);
private:
	void copyFrom(const Packet& packet);
	void autoFill();
};

class SmallPacket : public Packet<64> {};
class MediumPacket : public Packet<256> {};
class LargePacket : public Packet<1024> {};

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

/*template<size_t count>
Packet<count>::Packet(const char* string)
{
	autoFill();
#if _DEBUG
	assert(strlen(string) < count);
#endif
	strcpy(this->data(), string);
	this->back() = PacketType::NONE;
}

template<size_t count>
Packet<count>::Packet(const std::string& string)
{
	autoFill();
#if _DEBUG
	assert(string.size() < count);
#endif
	//No longer using null terminators as delimiters.
	//this->at(string.copy(this->data(), string.size())) = '\0';
	string.copy(this->data(), string.size());
	this->back() = PacketType::NONE;
}*/

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
	for (size_t i = 0; i < count; i++) {
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
	string.copy(reinterpret_cast<char* const>(this->data()), string.size());
}

template<size_t count>
PacketType Packet<count>::getType()
{
#if _DEBUG
	assert(this->back() < PacketType::COUNT);
#endif
	return this->back();
}

template<size_t count>
void Packet<count>::setType(PacketType packetType)
{
#if _DEBUG
	assert(packetType < PacketType::COUNT);
#endif
	this->back() = packetType;
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
