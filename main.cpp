// Currently, I have dynamic shader compilation working on Windows, but not on
// macOS.  If this is set to 0, you must do the following before executing:
// glslc shader.vert -o vert.spv
// glslc shader.frag -o frag.spv
#ifdef __APPLE__
#define PHALANX_DYNAMIC_SHADER_COMPILATION 0
#else
#define PHALANX_DYNAMIC_SHADER_COMPILATION 1
#endif


#include "camera.h"
#include "model.h"
#include "renderer.h"
#include "texture.h"
#include "vertex.h"
#include "window_handler.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>


void maybeLogFPS() {
	static auto lastPrintTime = std::chrono::steady_clock::now();
	static uint32_t fps = 0;

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
	try {
		WindowHandler windowHandler{};
		Camera camera{};

		// load the model
		Texture vikingRoomTexture = Texture::load("textures/viking_room.png");
		Model vikingRoomModel = Model::load("models/viking_room.obj");
		vikingRoomModel.texture = &vikingRoomTexture;

		Renderer renderer(&windowHandler, &camera, &vikingRoomModel);

		auto lastFrameTime = std::chrono::steady_clock::now();

		while (renderer.isRunning()) {
			windowHandler.pollEvents();
			renderer.draw();
			maybeLogFPS();

			auto now = std::chrono::steady_clock::now();
			auto frameDurationUs = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime);
			lastFrameTime = now;

			camera.update(windowHandler.getKeyStates(), windowHandler.getMouseState(), frameDurationUs);
		}

		renderer.cleanup();
		windowHandler.cleanup();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
