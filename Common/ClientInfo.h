#pragma once
#include "Address.h"

enum class ClientStatus : byte {
	FREE,
	IN_CHAT,
	IN_GAME,
	HELLA_LIT = 69
};

struct ClientDescriptor {
	size_t m_id = 0;
	ClientStatus m_status = ClientStatus::FREE;
	bool m_active = false;
};

struct ClientInformation {
	Address m_address;
	size_t m_id = 0;
	ClientStatus m_status = ClientStatus::FREE;
};
