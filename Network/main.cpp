#include "Network.h"
#include <iostream>

int main() {
	SmallPacket smallPacket;
	//There's some memory damage occuring to the last character. Investigate this after my ai midterm!
	smallPacket.fromString("Hello world from packet\n");
	MediumPacket mediumPacket;
	LargePacket largePacket;
	printf("%s", smallPacket.toString().c_str());
	return getchar();
}