#pragma once
typedef unsigned char byte;
namespace Packet {
	enum PacketType : byte {
		NONE = 0,
		CONNECT,
		QUIT,
		COUNT
	};

	template<typename T>
	T* Deserialize(const char* packet) { return reinterpret_cast<T*>(packet + 1); }

	template<typename T>
	void Serialize(char* packet, const T& object) {
		packet[0] = T::packetType();	//Static packetType() method.
		object.serialize(packet);		//Write raw data to packet data (implement struct Internal for such objects).
	}
}
