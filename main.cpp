// Currently, I have dynamic shader compilation working on Windows, but not on
// macOS.  If this is set to 0, you must do the following before executing:
// glslc shader.vert -o vert.spv
// glslc shader.frag -o frag.spv
#ifdef __APPLE__
#define PHALANX_DYNAMIC_SHADER_COMPILATION 0
#else
#define PHALANX_DYNAMIC_SHADER_COMPILATION 1
#endif


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
	// Texture statueTexture = Texture::load("textures/statue.jpg");
	// Model rectanglesModel;
	// rectanglesModel.texture = &statueTexture;
	// rectanglesModel.vertices = {
	// 	// first rectangle
	// 	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // top left
	// 	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // top right
	// 	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // bottom right
	// 	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // bottom left

	// 	// second rectangle
	// 	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // top left
	// 	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // top right
	// 	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // bottom right
	// 	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}} // bottom left
	// };
	// rectanglesModel.indices = {
	// 	0, 1, 2, 2, 3, 0, // first rectangle
	// 	4, 5, 6, 6, 7, 4 // second rectangle
	// };

	Texture vikingRoomTexture = Texture::load("textures/viking_room.png");
	Model vikingRoomModel = Model::load("models/viking_room.obj");
	vikingRoomModel.texture = &vikingRoomTexture;

	try {
		WindowHandler windowHandler{};
		Renderer renderer(&windowHandler, &vikingRoomModel);

		while (renderer.isRunning()) {
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
