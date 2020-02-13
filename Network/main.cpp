#include "Network.h"
#include <iostream>

int main() {
	SmallPacket smallPacket;
	smallPacket.fromString("Hello world from packet!\n");
	MediumPacket mediumPacket;
	LargePacket largePacket;
	printf("%s", smallPacket.toString().c_str());
	return getchar();
}
