#pragma once
#include "Address.h"
#include <utility>

enum ClientStatus : byte {
	FREE,
	IN_CHAT,
	IN_GAME
};

//Client descriptor.
struct ClientDesc {
	ClientStatus m_status = ClientStatus::FREE;
	bool m_active = false;
};

typedef std::pair<Address, ClientDesc> ClientInfo;

//struct ClientInfo {
//	Address m_address;
//	ClientDesc m_clientDesc;
//};