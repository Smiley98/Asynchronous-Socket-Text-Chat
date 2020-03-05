#pragma once
#include "Address.h"

enum ClientStatus : byte {
	FREE,
	IN_CHAT,
	IN_GAME
};

struct ClientDescriptor {
	ClientStatus m_status = ClientStatus::FREE;
	bool m_active = false;
};

//Makes deserialization less of a headache.
struct ClientInformation {
	Address m_address;
	ClientStatus m_status;
};
