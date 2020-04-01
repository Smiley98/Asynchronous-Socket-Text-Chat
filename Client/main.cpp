#include "Client.h"
#include "../Common/spritelib/spritelib.h"
#include "../Common/Timer.h"
#include "../Common/ClientInfo.h"
#include "../Common/NetworkObjects.h"
#include "../Common/Multicast.h"
#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <Windows.h>
#define LOGGING true
using namespace spritelib;

//Mouse x and y.
int mx = 0, my = 0;
bool moved = false;
void MouseFunc(Window::Button a_button, int a_mouseX, int a_mouseY, Window::EventType a_eventType)
{
	switch (a_eventType)
	{
	case Window::EventType::MouseMoved:
	{
		moved = true;
		mx = a_mouseX;
		my = a_mouseY;
	}
	break;
	//case Window::EventType::MouseButtonReleased:
	//{
	//
	//}
	//break;
	}
}

struct Particle {
	math::Vector3 position;
	math::Vector3 velocity;
};

int main() {
	Window& window = Window::get_game_window();
	window.init("MY GAME", /*1920, 1080*/640, 480)
		.set_mouse_callback(MouseFunc)
		.set_screen_size(/*1920, 1080*/640, 480)
		.set_clear_color(0, 255, 0);
	Text::load_font("../Common/assets/times.ttf", "TimesNewRoman");

	Client client;
	client.start();
	client.setState(ClientState::CONSUME);

	std::vector<Particle> particles(5);
	for (auto& particle : particles)
		particle.velocity = 5.0f;
	
	Timer networkTimer, updateTimer, latencyTimer, frameTimer;
	PacketBuffer incoming;

	std::vector<ClientInformation> allClientInfomration;
	ClientInformation thisClientInformation;

	Packet packet(PacketType::GENERIC, PacketMode::ONE_WAY);

	//Indicates whether this client updates the other client or vice-versa.
	bool master = false;
	//Sync status. Master sends to self (for an attempt at round-trip latency) and slave, slave just accepts.
	bool synced = false;

	//A hack to measure latency in attempt to sync.
	double latency = 0.0;
	double lag = 0.0;
	bool measuringLatency = false;

	//No frame limiting for now, just run as fast as possible so network latency is evident.
	double updateDelay = 0.0;

	uint64_t counter = 0;
	float dt1 = 0.0f, dt2 = 0.0f;
	float px = 0.0f, py = 0.0f;
	float vx = 0.0f, vy = 0.0f;

	while (true) {
		counter++;
		frameTimer.restart();
		//Do routine network stuff every 0.1 seconds.
		if (networkTimer.elapsed() >= 100.0 + lag) {
			networkTimer.restart();
			client.copyIncoming(incoming);

			packet = Packet(PacketType::THIS_CLIENT_INFORMATION, PacketMode::TWO_WAY);
			client.addOutgoing(packet);

			if (!measuringLatency) {
				packet = Packet(PacketType::LATENCY, PacketMode::TWO_WAY);
				client.addOutgoing(packet);
				measuringLatency = true;
				latencyTimer.restart();
			}

			if (moved) {
				moved = false;

			}

			//Deserialize all incoming packets.
			for (const Packet& i : incoming) {
#if LOGGING
				if (!i.typeString().empty()) {
					printf("Client received packet of type %s\n.", i.typeString().c_str());
				}
#endif
				switch (i.getType())
				{
				case PacketType::ALL_CLIENT_INFORMATION: {
					Packet::deserialize(i, allClientInfomration);
					break;
				}
				case PacketType::THIS_CLIENT_INFORMATION: {
					Packet::deserialize(i, thisClientInformation);
					break;
				}
				case PacketType::SYNC: {
					synced = true;
					break;
				}
				case PacketType::LATENCY: {
					latency = latencyTimer.elapsed();
					measuringLatency = false;
					break;
				}
				default:
					break;
				}
			}
			size_t lowest = std::numeric_limits<size_t>::max();
			for (const ClientInformation& clientInformation : allClientInfomration) {
				if (clientInformation.m_id < lowest)
					lowest = clientInformation.m_id;
			}
			if (thisClientInformation.m_id == lowest)
				master = true;
		}
		
		if (updateTimer.elapsed() >= updateDelay) {
			updateTimer.restart();
			counter % 2 == 0 ? dt1 = frameTimer.elapsed() : dt2 = frameTimer.elapsed();

			//Do latency compensation prediction (dead reckoning) if you're not the host.
			float dt = master ? dt2 - dt1 : (dt2 - dt1) + latency;
			//px = px + ax * dt;
			//py = py + ay * dt;
			printf("%f\n", dt);
			Shapes::set_color(1.0f, 0.0f, 0.0f);
			for (auto& particle : particles) {
				math::Vector3 target(mx, my);
				math::Vector3 direction = target.subtract(particle.position);
				direction = direction.normalize();
				//particle.position = particle.position.add(direction.multiply(particle.velocity.multiply(dt)));
				Shapes::draw_rectangle(true, particle.position.x, particle.position.y, 10.0f, 7.5f);
			}

			Shapes::set_color(1.0f, 1.0f, 1.0f);
			Shapes::draw_rectangle(true, mx, my, 50.0f, 37.5f);
			window.update();

			//Lag switches.
			if (GetAsyncKeyState(49)) {
				lag += 100.0;
				printf("New lag: %f.\n", lag);
			}
			else if (GetAsyncKeyState(50)) {
				lag -= 100.0;
				printf("New lag: %f.\n", lag);
			}

			//Sticking with the mouse.
			//if (GetAsyncKeyState(VK_LEFT)) {
			//
			//}
			//if (GetAsyncKeyState(VK_RIGHT)) {
			//
			//}
			//if (GetAsyncKeyState(VK_UP)) {
			//
			//}
			//if (GetAsyncKeyState(VK_DOWN)) {
			//
			//}
			if (GetAsyncKeyState(VK_ESCAPE)) {
				exit(0);
			}
		}
	}

	return getchar();
}