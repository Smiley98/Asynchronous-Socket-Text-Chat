#pragma once
#include <cassert>
#include <array>
#include <string>

typedef unsigned char byte;
enum PacketType : byte {
	NONE = 0,
	CONNECT,
	DISCONNECT,
	//Insert game-specific packet types here.
	COUNT
};

//class Network;
class ServerBase;
class ClientBase;
class Server;
class Client;
template<size_t count>
class PacketBase
{
	//friend Network;
	friend ServerBase;
	friend ClientBase;
	friend Server;
	friend Client;
public:
	PacketBase();
	PacketBase(PacketType packetType);
	PacketBase(const std::string& string);
	//PacketBase(const PacketBase<count>& packet);
	//PacketBase& operator=(const PacketBase<count>& packet);

	std::string toString() const;
	void fromString(const std::string& string);

	//Writes size amount of memory to dst from m_internal.raw.data() + offset.
	void read (      void* dst, size_t size = count, size_t offset = 0) const;

	//Writes size amount of memory to m_internal.raw.data() + offset from src.
	void write(const void* src, size_t size = count, size_t offset = 0);

	PacketType getType() const;
	void setType(PacketType packetType);
	std::string typeString() const;
	static size_t size();

protected:
	void init();
	//Unnecessary because this class is POD so the compiler generated stuff makes deep copies as desired.
	//void clone(const PacketBase& packet);

private:
	//Protected accessors can be overwritten if desired.
	struct Internal {
		PacketType m_type;
		std::array<byte, count> m_raw;
	} m_internal;

	const char* signedBytes() const;
	char* signedBytes();
	const byte* bytes() const;
	byte* bytes();
	const std::array<byte, count>& buffer() const;
	std::array<byte, count>& buffer();
};

//Constraining all sockets to sending packets containing raw data only of size 512.
class Packet :
	public PacketBase<512>
{
	//friend Network;
	friend Server;
	friend Client;

public:
	Packet() = default;
	Packet(PacketType packetType);
	Packet(const std::string& string);
	//Packet(const Packet& packet);
	//Packet& operator=(const Packet& packet);
};

template<size_t count>
PacketBase<count>::PacketBase()
{
	init();
}

template<size_t count>
inline PacketBase<count>::PacketBase(PacketType packetType)
{
	init();
	setType(packetType);
}

template<size_t count>
inline PacketBase<count>::PacketBase(const std::string& string)
{
	init();
	fromString(string);
}

//template<size_t count>
//PacketBase<count>::PacketBase(const PacketBase<count>& packet)
//{
//	clone(packet);
//}
//
//template<size_t count>
//PacketBase<count>& PacketBase<count>::operator=(const PacketBase<count>& packet)
//{
//	clone(packet);
//	return *this;
//}

template<size_t count>
inline std::string PacketBase<count>::toString() const
{
	char cstring[count + 1];
	memcpy(cstring, m_internal.m_raw.data(), count);
	cstring[count] = '\0';
	return std::string(cstring);
}

template<size_t count>
inline void PacketBase<count>::fromString(const std::string& string)
{
	assert(string.size() <= count);
	string.copy(reinterpret_cast<char* const>(m_internal.m_raw.data()), string.size());
}

template<size_t count>
inline void PacketBase<count>::read(void* dst, size_t size, size_t offset) const
{
	assert(size + offset <= count);
	memcpy(dst, m_internal.m_raw.data() + offset, size);
}

template<size_t count>
void PacketBase<count>::write(const void* src, size_t size, size_t offset)
{
	assert(size + offset <= count);
	memcpy(m_internal.m_raw.data() + offset, src, size);
}

template<size_t count>
inline PacketType PacketBase<count>::getType() const
{
	assert(m_internal.m_type < PacketType::COUNT);
	return m_internal.m_type;
}

template<size_t count>
inline void PacketBase<count>::setType(PacketType packetType)
{
	assert(packetType < PacketType::COUNT);
	m_internal.m_type = packetType;
}

template<size_t count>
inline std::string PacketBase<count>::typeString() const
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
size_t PacketBase<count>::size()
{
	return sizeof(Internal);
}

template<size_t count>
inline const char* PacketBase<count>::signedBytes() const
{
	return reinterpret_cast<const char*>(m_internal.m_raw.data());
}

template<size_t count>
inline char* PacketBase<count>::signedBytes()
{
	return reinterpret_cast<char*>(m_internal.m_raw.data());
}

template<size_t count>
inline const byte* PacketBase<count>::bytes() const
{
	return m_internal.m_raw.data();
}

template<size_t count>
inline byte* PacketBase<count>::bytes()
{
	return m_internal.m_raw.data();
}

template<size_t count>
inline const std::array<byte, count>& PacketBase<count>::buffer() const
{
	return m_internal.m_raw;
}

template<size_t count>
inline std::array<byte, count>& PacketBase<count>::buffer()
{
	return m_internal.m_raw
}

template<size_t count>
inline void PacketBase<count>::init()
{
	static_assert(count > 0);
	memset(&m_internal, 0, sizeof(Internal));
}

//template<size_t count>
//inline void PacketBase<count>::clone(const PacketBase<count>& packet)
//{
//	memcpy(&m_internal, &packet.m_internal, sizeof(Internal));
//}