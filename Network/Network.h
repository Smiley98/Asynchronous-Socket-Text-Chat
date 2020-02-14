#pragma once
#include <string>
#include <array>
#include <cassert>

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
	void write(const byte* src, size_t size);

	PacketType getType();
	void setType(PacketType packetType);
	std::string typeString();

protected:
	void init();
	void clone(const PacketBase& packet);

private:
	//Protected accessors can be overwritten if desired.
	struct Internal {
		PacketType m_type;
		std::array<byte, count> m_raw;
	} m_internal;
};

//Constraining all sockets to sending packets containing raw data only of size 512.
class Packet :
	public PacketBase<512>
{
	//Its annoying to have to overload these but that's the price I pay for using templates.
public:
	Packet();
	Packet(const Packet& packet);
	Packet& operator=(const Packet& packet);
};

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

class Network {
public:
	static void startupWSA();
	static void cleanupWSA();
};

template<size_t count>
PacketBase<count>::PacketBase()
{
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
	static_assert(count > 0);
	memset(&m_internal, 0, sizeof(Internal));
}

template<size_t count>
inline void PacketBase<count>::clone(const PacketBase<count>& packet)
{
	memcpy(&m_internal, &packet.m_internal, sizeof(Internal));
}
