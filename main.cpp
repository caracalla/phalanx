// Currently, I have dynamic shader compilation working on Windows, but not on
// macOS.  If this is set to 0, you must do the following before executing:
// glslc shader.vert -o vert.spv
// glslc shader.frag -o frag.spv
#ifdef __APPLE__
#define PHALANX_DYNAMIC_SHADER_COMPILATION 0
#else
#define PHALANX_DYNAMIC_SHADER_COMPILATION 1
#endif


#include "renderer.h"
#include "vertex.h"
#include "window_handler.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>


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



constexpr double pi = 3.14159265358979323846;
constexpr double one_twenty = 2 * pi / 3;
constexpr double two_forty = 2 * one_twenty;

double calculateAngle(std::chrono::microseconds frameDuration) {
	// the triangle should do a full rotation every 5 seconds
	constexpr int FIVE_SECONDS = 5 * 1000000;
	return 2 * pi * frameDuration.count() / FIVE_SECONDS;
}



int main() {
	// for maybeLogFPS
	lastPrintTime = std::chrono::steady_clock::now();

	auto lastFrameTime = std::chrono::steady_clock::now();
	double angle = 0.0;

	std::vector<Vertex> vertices = {
		// original triangle
		// {{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // top
		// {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}}, // right
		// {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}} // left

		// first rectangle
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // top left
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // top right
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // bottom right
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // bottom left

		// second rectangle
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // top left
		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // top right
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // bottom right
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}} // bottom left
	};

	std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0, // first rectangle
		4, 5, 6, 6, 7, 4 // second rectangle
	};

	try {
		WindowHandler windowHandler{};
		Renderer renderer(&windowHandler, &vertices, &indices);

		while (renderer.isRunning()) {
			// auto now = std::chrono::steady_clock::now();
			// auto frameDuration =
			// 		std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime);
			// lastFrameTime = now;

			// angle += calculateAngle(frameDuration);

			// // make it spin!
			// vertices[0].pos.x = sin(angle);
			// vertices[0].pos.y = cos(angle);

			// vertices[1].pos.x = sin(angle + two_forty);
			// vertices[1].pos.y = cos(angle + two_forty);

			// vertices[2].pos.x = sin(angle + one_twenty);
			// vertices[2].pos.y = cos(angle + one_twenty);

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
