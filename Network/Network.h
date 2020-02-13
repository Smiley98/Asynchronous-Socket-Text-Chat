#pragma once
#include <string>

typedef unsigned char byte;
enum PacketType : byte {
	NONE = 0,
	CONNECT,
	QUIT,
	COUNT
};

template<size_t count>
class PacketBase
{
public:
	PacketBase();
	PacketBase(const PacketBase<count>& packet);
	PacketBase& operator=(const PacketBase<count>& packet);

	std::string toString();
	void fromString(const std::string& string);
	
	void read(byte* dst, size_t range = count, size_t offset = 0);
	void write(const byte* src, size_t size);	//Writes the contents of bytes through bytes + size to the internal buffer.

	PacketType getType();
	void setType(PacketType packetType);
	std::string typeString();

private:
	struct Internal {
		PacketType m_type;
		std::array<byte, count> m_raw;
	} m_internal;

	void init();
	void clone(const PacketBase& packet);
};

class Packet : public PacketBase<512> {};

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

template<size_t count>
PacketBase<count>::PacketBase()
{
	static_assert(count > 0);
	init();
}

template<size_t count>
PacketBase<count>::PacketBase(const PacketBase<count>& packet)
{
	clone(packet);
}

template<size_t count>
PacketBase<count>& PacketBase<count>::operator=(const PacketBase<count>& packet)
{
	clone(packet);
	return *this;
}

template<size_t count>
std::string PacketBase<count>::toString()
{
	return std::string(reinterpret_cast<const char*>(m_internal.m_raw.data()));
}

template<size_t count>
void PacketBase<count>::fromString(const std::string& string)
{
	assert(string.size() <= count);
	string.copy(reinterpret_cast<char* const>(m_internal.m_raw.data()), string.size());
}

template<size_t count>
void PacketBase<count>::read(byte* dst, size_t range, size_t offset)
{
	assert(range + offset <= count);
	memcpy(dst, m_internal.m_raw.data() + offset, range);
}

template<size_t count>
void PacketBase<count>::write(const byte* bytes, size_t size)
{
	assert(size <= count);
	memcpy(m_internal.m_raw.data(), bytes, size);
}

template<size_t count>
PacketType PacketBase<count>::getType()
{
	assert(m_internal.m_type < PacketType::COUNT);
	return m_internal.m_type;
}

template<size_t count>
void PacketBase<count>::setType(PacketType packetType)
{
	assert(packetType < PacketType::COUNT);
	m_internal.m_type = packetType;
}

template<size_t count>
std::string PacketBase<count>::typeString()
{
	switch (getType())
	{
		case PacketType::NONE:
			return "none";
		case PacketType::CONNECT:
			return "connect";
		case PacketType::QUIT:
			return "quit";
		default:
			return "";
	}
}

template<size_t count>
inline void PacketBase<count>::init()
{
	memset(&m_internal, 0, sizeof(Internal));
}

template<size_t count>
inline void PacketBase<count>::clone(const PacketBase<count>& packet)
{
	memcpy(&m_internal, &packet.m_internal, sizeof(Internal));
}

//template<typename T>
//T* Deserialize(const char* packet) { return reinterpret_cast<T*>(packet + 1); }
//
//template<typename T>
//void Serialize(char* packet, const T& object) {
//	packet[0] = T::packetType();	//Static packetType() method.
//	object.serialize(packet);		//Write raw data to packet data (implement struct Internal for such objects).
//}

