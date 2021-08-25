#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <utility>
#include <vector>


const int INITIAL_WINDOW_WIDTH = 800;
const int INITIAL_WINDOW_HEIGHT = 600;


struct KeyStates {
	bool forward = false;
	bool reverse = false;
	bool left = false;
	bool right = false;
	bool rise = false;
	bool fall = false;
};

struct MouseState {
	double xOffset = 0;
	double yOffset = 0;

	void reset() {
		// prevent drift from old inputs sticking around
		xOffset = 0;
		yOffset = 0;
	}
};


double mouseLastXpos = INITIAL_WINDOW_WIDTH / 2;
double mouseLastYpos = INITIAL_WINDOW_HEIGHT / 2;
bool firstMouseMovement = true;


struct WindowHandler {
	static void framebufferResizeCallback(
				GLFWwindow* window, int width, int height) {
		auto self = reinterpret_cast<WindowHandler*>(
				glfwGetWindowUserPointer(window));
		self->framebufferResized_ = true;

		// reset last mouse position
		mouseLastXpos = width / 2;
		mouseLastYpos = height / 2;
	}

	static void keyPressCallback(
				GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto self = reinterpret_cast<WindowHandler*>(
				glfwGetWindowUserPointer(window));

		bool pressed = action != GLFW_RELEASE;

		switch(key) {
			case GLFW_KEY_W:
				self->keyStates_.forward = pressed;
				break;
			case GLFW_KEY_S:
				self->keyStates_.reverse = pressed;
				break;
			case GLFW_KEY_A:
				self->keyStates_.left = pressed;
				break;
			case GLFW_KEY_D:
				self->keyStates_.right = pressed;
				break;
			case GLFW_KEY_Q:
				self->keyStates_.rise = pressed;
				break;
			case GLFW_KEY_E:
				self->keyStates_.fall = pressed;
				break;
			default:
				break;
		}

		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			std::cout << "pressed the r key\n";
			self->shouldRecreateSwapChain_ = true;
		}
	}

	static void mousePositionCallback(
			GLFWwindow* window, double xpos, double ypos) {
		if (firstMouseMovement) {
			mouseLastXpos = xpos;
			mouseLastYpos = ypos;
			firstMouseMovement = false;
			return;
		}

		auto self = reinterpret_cast<WindowHandler*>(
				glfwGetWindowUserPointer(window));

		self->mouseState_.xOffset = (xpos - mouseLastXpos) / 10;
		self->mouseState_.yOffset = (ypos - mouseLastYpos) / 10;

		mouseLastXpos = xpos;
		mouseLastYpos = ypos;
	}

	WindowHandler() {
		this->window_ = this->createWindow();

		glfwSetWindowUserPointer(window_, this);
		glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
		glfwSetKeyCallback(window_, keyPressCallback);

		// capture mouse
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window_, mousePositionCallback);
	}

	GLFWwindow* createWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		return glfwCreateWindow(
			INITIAL_WINDOW_WIDTH,
			INITIAL_WINDOW_HEIGHT,
			"Phalanx",
			nullptr,
			nullptr);
	}

	bool wasWindowClosed() {
		return glfwWindowShouldClose(window_);
	}

	std::vector<const char*> getRequiredExtensions() {
		// this is where we actually get required extensions
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions =
			glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// beginning and end iterators of array
		std::vector<const char*> requiredExtensions(
			glfwExtensions,
			glfwExtensions + glfwExtensionCount);

		return requiredExtensions;
	}

	VkResult createWindowSurface(VkInstance& instance, VkSurfaceKHR* surfacePtr) {
		return glfwCreateWindowSurface(instance, window_, nullptr, surfacePtr);
	}

	bool framebufferWasResized() {
		return framebufferResized_;
	}

	void resetFramebufferResized() {
		framebufferResized_ = false;
	}

	bool shouldRecreateSwapchain() {
		return shouldRecreateSwapChain_;
	}

	void resetShouldRecreateSwapchain() {
		shouldRecreateSwapChain_ = false;
	}

	// Vulkan works with pixels, while GLFW's window size is measured in screen
	// coordinates.  On high DPI displays, those values won't be 1:1.
	// glfwGetFrameBufferSize returns the window dimensions in pixels
	std::pair<int, int> getFramebufferWidthHeight() {
		int width;
		int height;

		glfwGetFramebufferSize(window_, &width, &height);

		return std::pair<int, int>{width, height};
	}

	void pollEvents() {
		mouseState_.reset();
		glfwPollEvents();
	}

	void waitEvents() {
		glfwWaitEvents();
	}

	void cleanup() {
		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	KeyStates& getKeyStates() {
		return keyStates_;
	}

	MouseState& getMouseState() {
		return mouseState_;
	}

	GLFWwindow* window_;
	bool framebufferResized_ = false;
	bool shouldRecreateSwapChain_ = false;

	KeyStates keyStates_;
	MouseState mouseState_;
};
