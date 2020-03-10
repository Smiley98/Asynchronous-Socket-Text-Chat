#pragma once
#include "Address.h"

struct MulticastPacket {
	Packet m_packet;
	std::vector<Address> m_addresses;
};

namespace Multicast {
	//Forms a packet containing all addresses at the front followed by metadata and an object.
	template<typename T>
	Packet serialize(const std::vector<Address>& addresses, const T& object, PacketType packetType, PacketMode packetMode = PacketMode::ONE_WAY) {
		//1. Write the addresses to a multicasted packet.
		assert(addresses.size() > 0);
		Packet packet(PacketType::GENERIC, PacketMode::MULTICAST);
		Packet::serialize(addresses, packet);
		size_t dataIndex = sizeof(short) + sizeof(Address) * addresses.size();

		//2. Write the object count.
		const short objectCount = 1;
		packet.write(&objectCount, sizeof(short), dataIndex);
		dataIndex += sizeof(short);

		//3. Write the size of a single object.
		const short objectSize = sizeof(T);
		packet.write(&objectSize, sizeof(short), dataIndex);
		dataIndex += sizeof(short);

		//4. Write the metadata.
		packet.write(&packetType, sizeof(PacketType), dataIndex);
		dataIndex += sizeof(PacketType);
		packet.write(&packetMode, sizeof(PacketMode), dataIndex);
		dataIndex += sizeof(PacketMode);

		//5. Write the raw object.
		packet.write(&object, sizeof(object), dataIndex);

		return packet;
	}

	//Forms a packet containing all addresses at the front followed by metadata and objects.
	template<typename T>
	Packet serialize(const std::vector<Address>& addresses, const std::vector<T>& objects, PacketType packetType, PacketMode packetMode = PacketMode::ONE_WAY) {
		//1. Write the addresses to a multicasted packet.
		assert(addresses.size() > 0);
		assert(sizeof(T) * objects.size() <= Packet::bufferSize());
		Packet packet(PacketType::GENERIC, PacketMode::MULTICAST);
		Packet::serialize(addresses, packet);
		size_t dataIndex = sizeof(short) + sizeof(Address) * addresses.size();//2 + 2 * 20.

		//2. Write the number of objects.
		assert(objects.size() > 0);
		const unsigned short objectCount = objects.size();
		packet.write(&objectCount, sizeof(short), dataIndex);
		dataIndex += sizeof(short);

		//3. Write the size (in bytes) of a single object.
		const unsigned short objectSize = sizeof(T);
		packet.write(&objectSize, sizeof(short), dataIndex);
		dataIndex += sizeof(short);

		//4. Write the metadata.
		packet.write(&packetType, sizeof(PacketType), dataIndex);
		dataIndex += sizeof(PacketType);
		packet.write(&packetMode, sizeof(PacketMode), dataIndex);
		dataIndex += sizeof(PacketMode);

		//5. Write the raw objects.
		packet.write(objects.data(), objectSize * objectCount, dataIndex);

		return packet;
	}

	MulticastPacket deserialize(const Packet& packet) {
		//1. Extract address information.
		std::vector<Address> addresses;
		Packet::deserialize(packet, addresses);
		assert(addresses.size() > 0);
		size_t dataIndex = sizeof(short) + sizeof(Address) * addresses.size();

		//2. Extract object count.
		unsigned short objectCount = 0;
		packet.read(&objectCount, sizeof(short), dataIndex);
		assert(objectCount > 0);
		dataIndex += sizeof(short);

		//3. Extract object size.
		unsigned short objectSize = 0;
		packet.read(&objectSize, sizeof(short), dataIndex);
		assert(objectSize > 0);
		dataIndex += sizeof(short);

		//4. Extract packet metadata.
		PacketType packetType;
		packet.read(&packetType, sizeof(PacketType), dataIndex);
		dataIndex += sizeof(PacketType);
		PacketMode packetMode;
		packet.read(&packetMode, sizeof(PacketMode), dataIndex);
		dataIndex += sizeof(PacketMode);

		//5. Extract packet object data.
		Packet outgoing(packetType, packetMode);
		outgoing.write(&objectCount, sizeof(short));//Mimic deserialize function (can't actually use it cause we don't know the object type).
		outgoing.write(packet.buffer().data() + dataIndex, objectSize * objectCount, sizeof(short));

		return { outgoing, addresses };
	}
}

//template<typename T>
//Packet combine(const std::vector<Address>& addresses, const T& object, PacketType packetType, PacketMode packetMode = PacketMode::ONE_WAY) {
//	//The packet type at the start doesn't matter because the data after the addresses is what's multicasted.
//	Packet packet(PacketType::GENERIC, PacketMode::MULTICAST);
//	Packet::serialize(addresses, packet);
//	const size_t dataStart = 1 + sizeof(Address) * addresses.size();
//	//Write the number of trailing bytes.
//	packet.buffer()[dataStart] = sizeof(T) + Packet::headerSize();
//	packet.buffer()[dataStart + 1] = static_cast<byte>(packetType);
//	packet.buffer()[dataStart + 2] = static_cast<byte>(packetMode);
//	//After trailing bytes + metadata, write the object.
//	packet.write(&object, sizeof(object), dataStart + Packet::headerSize() + 1);
//	return packet;
//}