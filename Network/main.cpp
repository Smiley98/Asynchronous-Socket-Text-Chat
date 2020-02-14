#include "Network.h"
#include <iostream>

int main() {
	Packet packet;
	packet.fromString("Hello world from packet!\n");
	Packet packet2 = packet;

	packet2 = packet;
	printf("%s", packet2.toString().c_str());
	return getchar();
}
