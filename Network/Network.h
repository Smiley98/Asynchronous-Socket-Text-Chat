#pragma once
#include <winsock2.h>
#include <concurrent_vector.h>
#include <array>
#include <atomic>
#include <cassert>
#include <string>

typedef unsigned char byte;
typedef std::atomic_uchar atomic_byte;

//Packet memory is zero-initialized so it makes sense to represent none as 0.
enum PacketType : byte {
	NONE = 0,
	CONNECT,
	QUIT,
	COUNT
};

//Even this Packet structure is unnecessary! As long as we reserve element 0 for type and element 1+ for object we're set!
//struct Packet {
//	PacketType type;
//	void* data;
//};

template<typename T>
T* fromPacket(char* packet) { return reinterpret_cast<T*>(packet + 1); }
//toPacket is literally just assign the data member.

//Even this doesn't work because shit with overriden methods isn't POD. Just memcpy actual POD structures to/from packets!
//struct Raw {
//	//Writes data to packet.
//	virtual void Serialize(char* packet) = 0;
//
//	//Reads data from packet.
//	virtual void Deserialize(const char* packet) = 0;
//};

//Use case:
//byte packet[PACKET_SIZE];
//if not socket error,
//	switch(packet[0])
//		case INPUT:
//			network.inputs.push_back(*fromPacket<Input>(packet));//Dereference to ensure a deep copy is appended.

//This class is going to be ugly to use because its templated. Casting would created redundancies and look verbose.
/*template<size_t count>
class Packet
{
public:
	Packet();
	Packet(const Packet<count>& packet);
	Packet& operator=(const Packet<count>& packet);
	void copyToPool(concurrency::concurrent_vector<Packet<count>>& packetPool);

	std::string toString();
	void fromString(const std::string& string);

	PacketType getType();
	void setType(PacketType packetType);

private:
	//POD to ensure correct memory layout for serialization/deserialization.
	//*Cannot assign values on default initialization otherwise not POD.
	//*Class types call default constructor on default initialization, fundamental types are left uninitialized (so we gotta fill on packet create).
	struct Internal {
		PacketType m_type;
		std::array<byte, count> m_binary;
	} m_internal;

	void copyFrom(const Packet& packet);
	void zeroInitialize();
};

class SmallPacket : public Packet<63> {};
class MediumPacket : public Packet<255> {};
class LargePacket : public Packet<1023> {};

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
	zeroInitialize();
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
void Packet<count>::copyToPool(concurrency::concurrent_vector<Packet<count>>& packetPool)
{
	packetPool.resize(packetPool.size() + 1);
	packetPool.back() = *this;
}

template<size_t count>
std::string Packet<count>::toString()
{
	return std::string(reinterpret_cast<const char*>(m_internal.m_binary.data()));
}

template<size_t count>
void Packet<count>::fromString(const std::string& string)
{
	assert(string.size() < count);
	string.copy(reinterpret_cast<char* const>(m_internal.m_binary.data()), string.size());
}

template<size_t count>
PacketType Packet<count>::getType()
{
	assert(m_internal.m_type < PacketType::COUNT);
	return m_internal.m_type;
}

template<size_t count>
void Packet<count>::setType(PacketType packetType)
{
	assert(packetType < PacketType::COUNT);
	m_internal.m_type = packetType;
}

template<size_t count>
inline void Packet<count>::copyFrom(const Packet<count>& packet)
{
	memcpy(&m_internal, &packet.m_internal, sizeof(Internal));
}

template<size_t count>
inline void Packet<count>::zeroInitialize()
{
	memset(&m_internal, 0, sizeof(Internal));
}
*/