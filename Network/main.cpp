#include "Network.h"
#include <iostream>

int main() {
	Packet packet;
	packet.fromString("Hello world from packet!\n");
	printf("%s", packet.toString().c_str());
	return getchar();
}
