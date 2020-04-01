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

//Mouse x and y, delta mouse x and y.
int mx = 0, my = 0;
void MouseFunc(Window::Button a_button, int a_mouseX, int a_mouseY, Window::EventType a_eventType)
{
	switch (a_eventType)
	{
	case Window::EventType::MouseMoved:
	{
		mx = a_mouseX;
		my = a_mouseY;
	}
	break;
	}
}

int main() {
	Window& window = Window::get_game_window();
	window.init("MY GAME", /*1920, 1080*/640, 480)
		.set_mouse_callback(MouseFunc)
		.set_screen_size(/*1920, 1080*/640, 480)
		.set_clear_color(0, 255, 0);
	Text::load_font("../Common/assets/times.ttf", "TimesNewRoman");
	Shapes::set_color(1.0f, 1.0f, 1.0f);

	Client client;
	client.start();
	client.setState(ClientState::CONSUME);
	
	Timer networkTimer, latencyTimer, frameTimer, continuousTimer, correctionTimer;
	PacketBuffer incoming;

	std::vector<ClientInformation> allClientInfomration;
	ClientInformation thisClientInformation;

	Packet packet(PacketType::GENERIC, PacketMode::ONE_WAY);

	//Indicates whether this client updates the other client or vice-versa.
	bool host = false;

	//A hack to measure latency in attempt to sync.
	double latency = 0.0;
	double lag = 0.0;
	bool measuringLatency = false;

	//math::Vector3 position(320, 240);
	math::Vector3 localPosition(320, 240);
	math::Vector3 localVelocity;
	math::Vector3 localAcceleration;

	math::Vector3 remotePosition = localPosition;
	math::Vector3 remoteVelocity;
	math::Vector3 remoteAcceleration;
	bool kinematicFlag = false;

	const double velocityScalar = 75.0;
	const double accelerationScalar = 125.0;

	double frameTime = 0.0;
	while (true) {
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
				case PacketType::LATENCY: {
					latency = latencyTimer.elapsed();
					measuringLatency = false;
					break;
				}
				case PacketType::KINEMATIC: {
					Kinematic kinematic;
					Packet::deserialize(i, kinematic);
					//Snapping for now.
					remotePosition = kinematic.position;
					remoteVelocity = kinematic.velocity;
					remoteAcceleration = kinematic.acceleration;
					kinematicFlag = true;
					correctionTimer.restart();
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
				host = true;
		}
		
		//Game logic:
		window.update();

		//Do latency compensation prediction (dead reckoning) if you're not the host.
		float dt = host ? frameTime : frameTime + latency;

		math::Vector3 finalPosition;
		//Calculate physics based off mouse cursor if we're the host.
		if (host) {
			math::Vector3 target(mx, my);
			math::Vector3 direction = target.subtract(localPosition).normalize();

			localVelocity = direction.multiply(velocityScalar);
			localAcceleration.add(direction.multiply(accelerationScalar));

			math::Vector3 velocityComponent = localVelocity.multiply(dt);
			math::Vector3 accelerationComponent = localAcceleration.multiply(0.5 * dt * dt);

			localPosition = localPosition.add(velocityComponent.add(accelerationComponent));
			finalPosition = localPosition;

			packet = Packet(PacketType::KINEMATIC, PacketMode::REROUTE);
			Kinematic kinematic{ localPosition, localVelocity, localAcceleration };
			Packet::serialize(kinematic, packet);
			client.addOutgoing(packet);
		}
		
		//Otherwise, use the host's physics and account for latency.
		else {
			//Interpolate between local and remote over the course of latency milliseconds.
			math::Vector3 velocityComponent = remoteVelocity.multiply(dt);
			math::Vector3 accelerationComponent = remoteAcceleration.multiply(0.5 * dt * dt);
			remotePosition = remotePosition.add(velocityComponent.add(accelerationComponent));
			finalPosition = remotePosition;

			//This is probably flawed because we're not incrementing remotePosition.
			//In other words, I'm not good enough at programming to blend between local and remote for now ;)
			/*localPosition = remotePosition.add(velocityComponent.add(accelerationComponent));
			static double t = 0.0;
			if (kinematicFlag) {
				t = correctionTimer.elapsed() / latency;
				if (t >= 1.0)
					kinematicFlag = false;
			}
			finalPosition = math::Vector3::lerp(localPosition, remotePosition, t);*/
		}
		Shapes::draw_rectangle(true, finalPosition.x, finalPosition.y, 50.0f, 37.5f);

		//Uncomment to test locally.
		/*
		math::Vector3 target(mx, my);
		math::Vector3 direction = target.subtract(localPosition).normalize();

		localVelocity = direction.multiply(velocityScalar);
		localAcceleration.add(direction.multiply(accelerationScalar));

		math::Vector3 velocityComponent = localVelocity.multiply(dt);
		math::Vector3 accelerationComponent = localAcceleration.multiply(0.5 * dt * dt);

		localPosition = localPosition.add(velocityComponent.add(accelerationComponent));
		finalPosition = localPosition;

		packet = Packet(PacketType::KINEMATIC, PacketMode::REROUTE);
		Kinematic kinematic{ localPosition, localVelocity, localAcceleration };
		Packet::serialize(kinematic, packet);
		client.addOutgoing(packet);

		Shapes::draw_rectangle(true, finalPosition.x, finalPosition.y, 50.0f, 37.5f);
		//*/

		//Lag switches.
		if (GetAsyncKeyState(49)) {
			lag += 100.0;
			printf("New lag: %f.\n", lag);
		}
		else if (GetAsyncKeyState(50)) {
			lag -= 100.0;
			printf("New lag: %f.\n", lag);
		}

		if (GetAsyncKeyState(VK_ESCAPE)) {
			exit(0);
		}
		frameTime = frameTimer.elapsed();
	}

	return getchar();
}