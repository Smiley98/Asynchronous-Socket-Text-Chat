#pragma once
#include "Address.h"

enum class ClientStatus : byte {
	FREE,
	IN_CHAT,
	IN_GAME
};

struct ClientDescriptor {
	static size_t s_id;
	size_t m_id = 0;//Defaulted to 0 but never should always be 1 or greater when in use.
	ClientStatus m_status = ClientStatus::FREE;
	bool m_active = false;
};

//Makes deserialization less of a headache.
struct ClientInformation {
	Address m_address;
	size_t m_id = 0;
	ClientStatus m_status = ClientStatus::FREE;
};
