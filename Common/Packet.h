#pragma once
#include <cassert>
#include <array>
#include <string>

typedef unsigned char byte;
enum class PacketType : byte {
	NONE = 0,
	GENERIC,
	CONNECT,
	DISCONNECT,
	LIST_ALL_ACTIVE,
	STRING,
	//Insert game-specific packet types here.
	COUNT
};

enum class PacketMode : byte {
	NONE = 0,
	ONE_WAY,	//Server doesn't send the packet once received.
	TWO_WAY,	//Server sends back to sender once received.
	REROUTE,	//Server sends to everyone but the sender.
	BROADCAST,	//Server sends to everyone including the sender.
	SPECIFIC,	//Server sends to a specified list.
	COUNT
};

template<size_t count>
class PacketBase
{
public:
	PacketBase();
	PacketBase(PacketType packetType, PacketMode packetMode);
	PacketBase(const std::string& string, PacketMode packetMode = PacketMode::ONE_WAY);

	std::string toString() const;
	void fromString(const std::string& string);

	//Writes size amount of memory to dst from m_internal.raw.data() + offset.
	void read (      void* dst, size_t size = count, size_t offset = 0) const;

	//Writes size amount of memory to m_internal.raw.data() + offset from src.
	void write(const void* src, size_t size = count, size_t offset = 0);

	PacketType getType() const;
	void setType(PacketType packetType);
	std::string typeString() const;

	PacketMode getMode() const;
	void setMode(PacketMode packetMode);
	std::string modeString() const;

	static size_t size();
	static size_t rawSize();

	//I would leave these private and give friendship but then I lose auto-complete...
	const char* signedBytes() const;
	char* signedBytes();
	const byte* bytes() const;
	byte* bytes();
	const std::array<byte, count>& buffer() const;
	std::array<byte, count>& buffer();

protected:
	void init();

private:
	//Protected accessors can be overwritten if desired.
	struct Internal {
		PacketType m_type;
		PacketMode m_mode;
		std::array<byte, count> m_raw;
	} m_internal;
};

//Constraining all sockets to sending packets containing raw data only of size 512.
class Packet :
	public PacketBase<512>
{

public:
	Packet() = default;
	Packet(PacketType packetType, PacketMode packetMode);
	Packet(const std::string& string, PacketMode packetMode = PacketMode::ONE_WAY);
};

template<size_t count>
PacketBase<count>::PacketBase()
{
	init();
	setType(PacketType::GENERIC);
	setMode(PacketMode::ONE_WAY);
}

template<size_t count>
inline PacketBase<count>::PacketBase(PacketType packetType, PacketMode packetMode)
{
	init();
	setType(packetType);
	setMode(packetMode);
}

template<size_t count>
inline PacketBase<count>::PacketBase(const std::string& string, PacketMode packetMode)
{
	init();
	setType(PacketType::STRING);
	setMode(packetMode);
	fromString(string);
}

template<size_t count>
inline std::string PacketBase<count>::toString() const
{	//Can't strcpy because the internal array isn't guaranteed to have a null terminator.
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
{	//Consider implementing a mechanism to store the index of free data.
	assert(size + offset <= count);
	memcpy(m_internal.m_raw.data() + offset, src, size);
}

template<size_t count>
inline PacketType PacketBase<count>::getType() const
{
	assert(m_internal.m_type > PacketType::NONE && m_internal.m_type <= PacketType::COUNT);
	return m_internal.m_type;
}

template<size_t count>
inline void PacketBase<count>::setType(PacketType packetType)
{
	assert(packetType > PacketType::NONE && packetType <= PacketType::COUNT);
	m_internal.m_type = packetType;
}

template<size_t count>
inline std::string PacketBase<count>::typeString() const
{
	switch (getType())
	{
		case PacketType::GENERIC:
			return "generic";
		case PacketType::CONNECT:
			return "connect";
		case PacketType::DISCONNECT:
			return "disconnect";
		case PacketType::LIST_ALL_ACTIVE:
			return "active list";
		case PacketType::STRING:
			return "string";
		default:
			return "";
	}
}

template<size_t count>
PacketMode PacketBase<count>::getMode() const
{
	assert(m_internal.m_mode > PacketMode::NONE && m_internal.m_mode <= PacketMode::COUNT);
	return m_internal.m_mode;
}

template<size_t count>
void PacketBase<count>::setMode(PacketMode packetMode)
{
	assert(packetMode > PacketMode::NONE && packetMode < PacketMode::COUNT);
	m_internal.m_mode = packetMode;
}

template<size_t count>
std::string PacketBase<count>::modeString() const
{
	switch (getMode())
	{
		case PacketMode::ONE_WAY:
			return "one way";
		case PacketMode::TWO_WAY:
			return "two way";
		case PacketMode::REROUTE:
			return "reroute";
		case PacketMode::BROADCAST:
			return "broadcast";
		case PacketMode::SPECIFIC:
			return "specific";
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
inline size_t PacketBase<count>::rawSize()
{
	return m_internal.m_raw.size();
}

template<size_t count>
inline const char* PacketBase<count>::signedBytes() const
{
	return reinterpret_cast<const char*>(&m_internal);
}

template<size_t count>
inline char* PacketBase<count>::signedBytes()
{
	return reinterpret_cast<char*>(&m_internal);
}

template<size_t count>
inline const byte* PacketBase<count>::bytes() const
{
	return reinterpret_cast<const byte*>(&m_internal);
}

template<size_t count>
inline byte* PacketBase<count>::bytes()
{
	return reinterpret_cast<byte*>(&m_internal);
}

template<size_t count>
inline const std::array<byte, count>& PacketBase<count>::buffer() const
{
	return m_internal.m_raw;
}

template<size_t count>
inline std::array<byte, count>& PacketBase<count>::buffer()
{
	return m_internal.m_raw;
}

template<size_t count>
inline void PacketBase<count>::init()
{
	memset(&m_internal, 0, sizeof(Internal));
}