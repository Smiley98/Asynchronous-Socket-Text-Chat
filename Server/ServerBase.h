#pragma once
#include "../Network/Network.h"
#include <vector>

struct ClientDesc {
	SOCKADDR_IN m_address;
	int m_addressLength;
	bool operator==(const ClientDesc& desc) const;
};

//Note: this server has no "receive" functionality. Its meant as a middle-man between clients, so all it does is re-route.
class ServerBase :
	public Network
{
protected:
	//Initialize Winsock2 and setup a server socket.
	void init();

	//Cleanup Winsock2, server socket, and server address.
	void shutdown();

	//Route the incoming packet back to all clients but the passed in client (which is usually the original sender).
	void reroute(const Packet& packet, const ClientDesc& exemptClient, int flags = 0);

	//Send packet to all clients.
	void broadcast(const Packet& packet, int flags = 0);

	//Sends packet to specific client.
	void send(const Packet& packet, const ClientDesc& client, int flags = 0);

private:
	std::vector<ClientDesc> m_clients;
	ADDRINFO* m_address = NULL;
	SOCKET m_socket = INVALID_SOCKET;
};
