#include "Packet.h"

//Packet::Packet()
//{
//}

Packet::Packet(PacketType packetType) :
	PacketBase(packetType)
{
}

Packet::Packet(const std::string& string) :
	PacketBase(string)
{
}

//Packet::Packet(const Packet& packet) :
//	PacketBase(packet)
//{
//}
//
//Packet& Packet::operator=(const Packet& packet)
//{
//	PacketBase::operator=(packet);
//	return *this;
//}
