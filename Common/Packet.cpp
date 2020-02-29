#include "Packet.h"

Packet::Packet(PacketType packetType, PacketMode packetMode) :
	PacketBase(packetType, packetMode)
{
}

Packet::Packet(const std::string& string, PacketMode packetMode) :
	PacketBase(string, packetMode)
{
}
