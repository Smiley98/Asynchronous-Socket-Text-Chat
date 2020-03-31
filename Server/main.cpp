#include "Server.h"
#include "../Common/spritelib/spritelib.h"
#include "../Common/Address.h"

using namespace spritelib;
int main() {
	Server server;
	server.start();
	server.setState(ServerState::ROUTE);

	Window& window = Window::get_game_window();
	window.init("MY GAME", 1920, 1080)
		.set_screen_size(1920, 1080)
		//.set_keyboard_callback(KeyboardFunc)
		//.set_mouse_callback(MouseFunc)
		.set_clear_color(0, 255, 255);
	Text::load_font("../Common/assets/times.ttf", "TimesNewRoman");

	//3 and 4 on the server because GetAsyncKeyState isn't application-specific. Also its extremely sensitive.
	while (true) {
		if (GetAsyncKeyState(51))
			server.addLag(0.1);
		if (GetAsyncKeyState(52))
			server.addLag(-0.1);

		window.update();
		Shapes::draw_rectangle(true, (float)(rand() % 1820), (float)(rand() % 1000), 100.0f, 80.0f);
	}

	return getchar();
}
