#include "Network.h"
#include <iostream>

struct Meme {
	char a = 65;
	void print() {
		printf("%c %c %c\n", a, b, c);
	}
	char b = 66;
	char c = 67;
};

int main() {
	Packet packet;
	packet.fromString("Hello world from packet!\n");
	printf("%s", packet.toString().c_str());

	Meme meme;
	Packet packet2;
	packet2.write(&meme, sizeof(Meme));

	memset(&meme, 0, sizeof(Meme));
	meme.print();
	packet2.read(&meme, sizeof(Meme));
	meme.print();

	printf("%s\n", packet2.toString().c_str());
	return getchar();
}
