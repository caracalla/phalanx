#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <utility>
#include <vector>


const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 400;


struct WindowHandler {
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto self = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
		self->framebufferResized_ = true;
	}

	static void keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto self = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));

		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			std::cout << "pressed the r key\n";
			self->shouldRecreateSwapChain_ = true;
		}
	}

	WindowHandler() {
		this->window_ = this->createWindow();

		glfwSetWindowUserPointer(window_, this);
		glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
		glfwSetKeyCallback(window_, keyPressCallback);
	}

	GLFWwindow* createWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		return glfwCreateWindow(
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
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
		if (shouldRecreateSwapChain_) {
			std::cout << "it's true\n";
		}
		return shouldRecreateSwapChain_;
	}

	void resetShouldRecreateSwapchain() {
		std::cout << "resetting the value\n";
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
		glfwPollEvents();
	}

	void waitEvents() {
		glfwWaitEvents();
	}

	void cleanup() {
		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	GLFWwindow* window_;
	bool framebufferResized_ = false;
	bool shouldRecreateSwapChain_ = false;
};
