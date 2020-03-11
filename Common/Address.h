#pragma once
#include "Packet.h"
#include <WinSock2.h>

struct Address;
uint64_t hashAddress(const Address& address);
bool compareAddresses(const SOCKADDR_IN& a, const SOCKADDR_IN& b);

struct Address {

	Address();
	SOCKADDR_IN m_sai;
	int m_length;
	bool operator==(const Address& address) const;

	bool sendTo(SOCKET soc, const Packet& packet) const;
	bool recvFrom(SOCKET soc, Packet& packet);

	void print() const;
};

struct AddressHash {
	uint64_t operator()(const Address& address) const;
};