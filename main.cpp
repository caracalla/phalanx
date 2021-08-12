#include "renderer.h"
#include "window_handler.h"

#include <chrono>
#include <iostream>
#include <stdexcept>


uint32_t fps = 0;
std::chrono::steady_clock::time_point lastPrintTime;

void maybeLogFPS() {
	fps++;
	auto now = std::chrono::steady_clock::now();
	auto elapsedTime = now - lastPrintTime;

	if (elapsedTime >= std::chrono::seconds(1)) {
		std::cout << "FPS: " << fps << std::endl;
		lastPrintTime = now;
		fps = 0;
	}
}



int main() {
	// for maybeLogFPS
	lastPrintTime = std::chrono::steady_clock::now();

	uint32_t spinFactor = 0;
	double pi = 3.14159265358979323846;

	double one_twenty = 2 * pi / 3;
	double two_forty = 2 * one_twenty;

	try {
		WindowHandler windowHandler{};
		Renderer renderer(&windowHandler);

		while (renderer.isRunning()) {
			float angle = spinFactor / 2000.0;
			spinFactor++;

			// make it spin!
			vertices[0].pos.x = sin(angle);
			vertices[0].pos.y = cos(angle);

			vertices[1].pos.x = sin(angle + two_forty);
			vertices[1].pos.y = cos(angle + two_forty);

			vertices[2].pos.x = sin(angle + one_twenty);
			vertices[2].pos.y = cos(angle + one_twenty);

			windowHandler.pollEvents();
			renderer.draw();
			maybeLogFPS();
		}

		renderer.cleanup();
		windowHandler.cleanup();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
