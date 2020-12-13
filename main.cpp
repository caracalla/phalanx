#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/vec4.hpp>
// #include <glm/mat4x4.hpp>
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <thread>
#include <vector>




struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};

std::vector<Vertex> vertices = {
	{{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}
};



// file loading helper, for shaders
static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

// we compile shaders on the fly, before rendering begins
std::vector<char> loadShader(
		const std::string& shaderFileName,
		shaderc_shader_kind shaderKind) {
	std::vector<char> shaderGlsl = readFile(shaderFileName);

	if (shaderGlsl.empty()) {
		throw std::runtime_error("empty shader file provided");
	}

	shaderc::CompileOptions compileOptions;
	compileOptions.SetSourceLanguage(shaderc_source_language_glsl);

	const char* entryPointName = "main";

	shaderc::Compiler compiler;
	const auto compilationResult = compiler.CompileGlslToSpv(
			shaderGlsl.data(),
			shaderGlsl.size(),
			shaderKind,
			shaderFileName.c_str(),
			entryPointName,
			compileOptions);

	if (
			compilationResult.GetCompilationStatus() !=
			shaderc_compilation_status_success) {
		std::cerr << "shader compilation error:\n";
		std::cerr << compilationResult.GetErrorMessage() << std::endl;
		
		throw std::runtime_error("failed to compile shader");
	}

	std::vector<char> compilationOutput(
			reinterpret_cast<const char*>(compilationResult.cbegin()),
			reinterpret_cast<const char*>(compilationResult.cend()));

	return compilationOutput;
}

std::vector<char> loadVertexShader(const std::string& shaderFileName) {
	return loadShader(shaderFileName, shaderc_glsl_default_vertex_shader);
}

std::vector<char> loadFragmentShader(const std::string& shaderFileName) {
	return loadShader(shaderFileName, shaderc_glsl_default_fragment_shader);
}



// debug messenger creation and destruction function loading
VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		std::cout << "couldn't find create debug utils function\n\n";
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	} else {
		std::cout << "couldn't find destroy debug utils function\n\n";
	}
}



struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};





const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 400;

const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};





struct Renderer {
	static Renderer create() {
		Renderer renderer{};

		// doesn't actually do anything for now

		return renderer;
	}

	void init() {
		window_ = createWindow();

		glfwSetWindowUserPointer(window_, this);
		glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
		glfwSetKeyCallback(window_, keyPressCallback);

		initVulkan();
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

	// GLFW callbacks
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto self = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
		self->framebufferResized_ = true;
	}

	static void keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto self = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));

		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			std::cout << "pressed the r key\n";
			self->shouldRecreateSwapChain_ = true;
		}
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFrameBuffers();
		createCommandPool();
		createVertexBuffer();
		createCommandBuffers();
		createSyncObjects();
	}

	bool isRunning() {
		return !glfwWindowShouldClose(window_);
	}

	void draw() {
		glfwPollEvents();

		if (shouldRecreateSwapChain_) {
			recreateSwapChain("user pressed the R key");
			shouldRecreateSwapChain_ = false;
		}

		drawFrame();
	}

	void drawFrame() {
		// to prevent more than MAX_FRAMES_IN_FLIGHT frames from being submitted,
		// which could cause a new frame to use objects already in use by an
		// in-flight previous frame, we use a fence for each frame to prevent
		// oversubmission
		vkWaitForFences(
				logicalDevice_,
				1,
				&inFlightFences_[currentFrame_],
				VK_TRUE, // wait for all fences to be finished, although we're only passing in one here
				UINT64_MAX);

		size_t memSize = sizeof(vertices[0]) * vertices.size();
		memcpy(vertexData_, vertices.data(), memSize);

		uint32_t imageIndex;
		VkResult acquireImageResult = vkAcquireNextImageKHR(
				logicalDevice_,
				swapChain_,
				UINT64_MAX, // timeout, disabled
				imageAvailableSemaphores_[currentFrame_],
				VK_NULL_HANDLE, // fence, if we were using one
				&imageIndex);

		if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
			// not possible to present to an out-of-date swap chain, so recreate
			// and try again
			recreateSwapChain("swap chain out of date when acquiring image");
			return;
		} else if (
				acquireImageResult != VK_SUCCESS &&
				acquireImageResult != VK_SUBOPTIMAL_KHR) {
			// suboptimal is still ok
			throw std::runtime_error("failed to acquire swap chain image");
		}

		// if MAX_FRAMES_IN_FLIGHT is less than swapChainImages_.size(), or
		// vkAcquireNextImageKHR returns images out of order, it would be possible
		// to start rendering to swap chain images that are already in flight

		// we check if a previous frame is already using this image by checking the
		// corresponding frame fence
		if (imagesInFlight_[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(
					logicalDevice_,
					1,
					&imagesInFlight_[imageIndex],
					VK_TRUE,
					UINT64_MAX);
		}

		imagesInFlight_[imageIndex] = inFlightFences_[currentFrame_];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// wait to write colors to image until it's available
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores_[currentFrame_] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers_[imageIndex];

		// signals when command buffer has finished execution
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores_[currentFrame_] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// fences need to be manually restored to the unsignalled state
		vkResetFences(logicalDevice_, 1, &inFlightFences_[currentFrame_]);

		// when the frame is submitted, the fence will be signalled
		if (
				vkQueueSubmit(
						graphicsQueue_,
						1,
						&submitInfo,
						inFlightFences_[currentFrame_]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores; // wait until render is finished to present

		VkSwapchainKHR swapChains[] = { swapChain_ };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // optional

		VkResult queuePresentResult = vkQueuePresentKHR(presentQueue_, &presentInfo);

		if (
				queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR ||
				queuePresentResult == VK_SUBOPTIMAL_KHR ||
				framebufferResized_) {
			std::string reason =
					framebufferResized_ ?
							"frame buffer was resized" :
							"presenting image: swap chain out of date or suboptimal";
			framebufferResized_ = false;
			recreateSwapChain(reason);
		}
		else if (queuePresentResult != VK_SUCCESS) {
			throw std::runtime_error("failed to acquire swap chain image");
		}

		// wait for the submitted work to finish (not performant, prevents rendering
		// more than one frame at a time)
		// vkQueueWaitIdle(presentQueue_);

		currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void cleanup() {
		// wait for the logical device to finish all operations before cleaning up
		vkDeviceWaitIdle(logicalDevice_);

		vkUnmapMemory(logicalDevice_, vertexBufferMemory_);
		cleanupSwapChain();

		vkDestroyBuffer(logicalDevice_, vertexBuffer_, nullptr);
		vkFreeMemory(logicalDevice_, vertexBufferMemory_, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(logicalDevice_, renderFinishedSemaphores_[i], nullptr);
			vkDestroySemaphore(logicalDevice_, imageAvailableSemaphores_[i], nullptr);
			vkDestroyFence(logicalDevice_, inFlightFences_[i], nullptr);
		}

		vkDestroyCommandPool(logicalDevice_, commandPool_, nullptr);
		vkDestroyDevice(logicalDevice_, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
		}

		vkDestroySurfaceKHR(instance_, surface_, nullptr);
		vkDestroyInstance(instance_, nullptr);

		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	// **************************************************************************
	// * Instance
	// **************************************************************************

	// wrapper around vkCreateInstance
	// also logs available extensions
	void createInstance() {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("couldn't find requested validation layers");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Phalanx";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// get required extensions
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// enable validation layers
		// create debug messenger info outside of if to prevent being destroyed
		// before the call to vkCreateInstance()
		auto debugMessengerInfo = createDebugMessengerInfo();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount =
					static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			// why is the cast necessary here? it should already have that type
			// createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugMessengerInfo;
			createInfo.pNext = &debugMessengerInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create VkInstance");
		}

		// this is just a print for my own illumination
		// get enabled extensions
		uint32_t availableExtensionCount = 0;
		// get the number of extensions first
		vkEnumerateInstanceExtensionProperties(
				nullptr,
				&availableExtensionCount,
				nullptr);
		std::vector<VkExtensionProperties> availableExtensions(
				availableExtensionCount);
		// actually get the extension properties
		vkEnumerateInstanceExtensionProperties(
			nullptr,
			&availableExtensionCount,
			availableExtensions.data());

		std::cout << "\n\n";
		std::cout << "available extensions:\n";

		for (const auto& extension : availableExtensions) {
			std::cout << "    " << extension.extensionName << std::endl;
		}

		std::cout << "\n\n";
	}

	bool checkValidationLayerSupport() {
		// get the number of available layers first
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		
		// actually get the layer properties
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		std::cout << "\n\n";
		std::cout << "available layers:\n";

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				std::cout << "    " << layerProperties.layerName << std::endl;
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				std::cout << "could not find validation layer " << layerName << std::endl;
				return false;
			}
		}

		std::cout << "\n\n";

		return true;
	}

	// gets and prints required GLFW extensions
	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions =
				glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// beginning and end iterators of array
		std::vector<const char*> extensions(
				glfwExtensions,
				glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// print required extensions
		std::cout << "\n\n";
		std::cout << "glfw required extensions:\n";

		for (const auto& extension : extensions) {
			std::cout << "    " << extension << std::endl;
		}

		std::cout << "\n\n";

		return extensions;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT createDebugMessengerInfo() {
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			// VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional

		return createInfo;
	}

	void setupDebugMessenger() {
		if (!enableValidationLayers) {
			return;
		}

		auto createInfo = createDebugMessengerInfo();

		if (
				CreateDebugUtilsMessengerEXT(
						instance_, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS) {
			throw std::runtime_error("failed to setup debug messenger");
		}
	}

	void createSurface() {
		if (
				glfwCreateWindowSurface(
						instance_, window_, nullptr, &surface_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	// **************************************************************************
	// * Physical Device
	// **************************************************************************

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

		// list the available devices
		std::cout << "\n\n";
		std::cout << "available devices (" << deviceCount << "):\n";

		for (const auto& device : devices) {
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			std::cout << "    " << deviceProperties.deviceName << std::endl;
		}

		std::cout << "\n\n";

		// pick the best device
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice_ = device;
				break;
			}
		}

		if (physicalDevice_ == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU");
		}
		else {
			std::cout << "\n\n";
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice_, &deviceProperties);
			std::cout << "using device " << deviceProperties.deviceName << "\n\n\n";
		}
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		// we would use this if we wanted to specifically use a discrete GPU
		// VkPhysicalDeviceProperties deviceProperties;
		// vkGetPhysicalDeviceProperties(device, &deviceProperties);

		// VkPhysicalDeviceFeatures deviceFeatures;
		// vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		// return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		// 		deviceFeatures.geometryShader;

		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			auto swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate =
					!swapChainSupport.formats.empty() &&
					!swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices{};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(
				device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(
				device,
				&queueFamilyCount,
				queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(
				device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(
				device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(
				deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	// **************************************************************************
	// * Logical Device
	// **************************************************************************

	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};

		std::cout << "\n\n";
		std::cout << "graphics family index: " << indices.graphicsFamily.value() << std::endl;
		std::cout << "present family index: " << indices.presentFamily.value() << std::endl;
		std::cout << "\n\n";

		float queuePriority = 1.0f; // pretending this is an array
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount =
				static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// newer implementations of Vulkan will ignore this, but we're including it
		// for completeness I guess
		if (enableValidationLayers) {
			createInfo.enabledLayerCount =
					static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &logicalDevice_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device");
		}

		vkGetDeviceQueue(
				logicalDevice_,
				indices.graphicsFamily.value(), // queue family index
				0, // queue index
				&graphicsQueue_);

		vkGetDeviceQueue(
				logicalDevice_,
				indices.presentFamily.value(), // queue family index
				0, // queue index
				&presentQueue_);
	}

	// **************************************************************************
	// * Swap Chain
	// **************************************************************************

	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport =
				querySwapChainSupport(physicalDevice_);

		VkSurfaceFormatKHR surfaceFormat =
				chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode =
				chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// get one more than the minimum, to avoid waiting on the driver
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (
				swapChainSupport.capabilities.maxImageCount > 0 &&
				imageCount > swapChainSupport.capabilities.maxImageCount) {
			// tested locally, the max on my machine is 8, min is 2
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface_;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // always 1 except for stereoscopic 3D
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);
		uint32_t queueFamilyIndices[] = {
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // optional, must be greater than 1 if using concurrent sharing mode
			createInfo.pQueueFamilyIndices = nullptr; // optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE; // assuming we only ever create one

		if (
				vkCreateSwapchainKHR(
						logicalDevice_, &createInfo, nullptr, &swapChain_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain");
		}

		uint32_t actualImageCount = 0;
		vkGetSwapchainImagesKHR(
				logicalDevice_, swapChain_, &actualImageCount, nullptr);
		swapChainImages_.resize(actualImageCount);
		vkGetSwapchainImagesKHR(
				logicalDevice_, swapChain_, &actualImageCount, swapChainImages_.data());

		swapChainImageFormat_ = surfaceFormat.format;
		swapChainExtent_ = extent;
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details{};

		// populate capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			device, surface_, &details.capabilities);

		// populate formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device, surface_, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				device, surface_, &formatCount, details.formats.data());
		}

		// populate presentModes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, surface_, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device, surface_, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& format : availableFormats) {
			if (
				format.format == VK_FORMAT_B8G8R8A8_SRGB &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return format;
			}
		}

		// couldn't find a preferred one, just return the first one
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes) {
		// mailbox is preferred, for triple buffering
		for (const auto& presentMode : availablePresentModes) {
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return presentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		} else {
			int width;
			int height;
			glfwGetFramebufferSize(window_, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(
					capabilities.minImageExtent.width,
					std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void cleanupSwapChain() {
		for (auto framebuffer : swapChainFramebuffers_) {
			vkDestroyFramebuffer(logicalDevice_, framebuffer, nullptr);
		}

		// not recreating command pool, because it is wasteful (?)
		vkFreeCommandBuffers(
			logicalDevice_,
			commandPool_,
			static_cast<uint32_t>(commandBuffers_.size()),
			commandBuffers_.data());

		vkDestroyPipeline(logicalDevice_, graphicsPipeline_, nullptr);
		vkDestroyPipelineLayout(logicalDevice_, pipelineLayout_, nullptr);
		vkDestroyRenderPass(logicalDevice_, renderPass_, nullptr);

		for (auto imageView : swapChainImageViews_) {
			vkDestroyImageView(logicalDevice_, imageView, nullptr);
		}

		vkDestroySwapchainKHR(logicalDevice_, swapChain_, nullptr);
	}

	void recreateSwapChain(std::string reason) {
		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(window_, &width, &height);

		if (width == 0 || height == 0) {
			while (width == 0 || height == 0) {
				std::cout << "window is minimized, waiting\n";
				glfwGetFramebufferSize(window_, &width, &height);
				glfwWaitEvents();
			}

			std::cout << "window unminimized\n\n";
		}

		vkDeviceWaitIdle(logicalDevice_);

		std::cout << "recreating swap chain: " << reason << std::endl;

		cleanupSwapChain();

		createSwapChain();
		createImageViews(); // based directly on swap chain images
		createRenderPass(); // depends on swap chain format (probably won't change, but handle it anyways)
		createGraphicsPipeline(); // depends on viewport and scissor sizes (unless using dynamic state)
		createFrameBuffers(); // depends on swap chain images
		// createCommandPool(); // don't need to recreate, can just reuse to recreate commad buffers
		createCommandBuffers(); // depends on swap chain images
	}

	// **************************************************************************
	// * Image Views
	// **************************************************************************

	void createImageViews() {
		swapChainImageViews_.resize(swapChainImages_.size());

		for (size_t i = 0; i < swapChainImages_.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages_[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat_;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (
					vkCreateImageView(
							logicalDevice_,
							&createInfo,
							nullptr,
							&swapChainImageViews_[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views");
			}
		}
	}

	// **************************************************************************
	// * Render Pass
	// **************************************************************************

	void createRenderPass() {
		// create single color buffer attachment represented by one image from the
		// swap chain
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat_;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling yet
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear the framebuffer to black before drawing a new frame
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store rendered contents in memory
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // not using stencil buffer
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // not using stencil buffer
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; // only have a single attachment, index 0
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;

		// set up a single render subpass
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef; // this is referenced in the fragment shader by layout(location = 0)

		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		// create a subpass dependency so the render pass waits until the image is available
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // tells us this is an implicit subpass; since it's the srcSubpass, tells us that it's the pre-render subpass
		dependency.dstSubpass = 0; // refers to this subpass, which is the only one (there's an implicit one at the end of the render pass)
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // what we're waiting on
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // the stage...
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // and action that we are waiting before

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (
				vkCreateRenderPass(
						logicalDevice_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass");
		}
	}

	// **************************************************************************
	// * Graphics Pipeline
	// **************************************************************************

	void createGraphicsPipeline() {
		// set up vertex and fragment shaders
		// auto vertShaderIRCode = readFile("vert.spv");
		// auto fragShaderIRCode = readFile("frag.spv");

		auto vertShaderIRCode = loadVertexShader("shader.vert");
		auto fragShaderIRCode = loadFragmentShader("shader.frag");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderIRCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderIRCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {
			vertShaderStageInfo,
			fragShaderStageInfo
		};

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		// set up vertex input (dummy for now, since vertices are defined directly in
		// the vertex shader
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount =
				static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		
		// set up input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// set up viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent_.width;
		viewport.height = (float)swapChainExtent_.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent_;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// set up rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // LINE or POINT requires enabling a GPU feature
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // cull back faces
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // specify vertex order for faces to be considered front faces
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 1.0f; // optional
		rasterizer.depthBiasClamp = 0.0f; // optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // optional

		// set up multisampling, disabled for now
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // optional
		multisampling.pSampleMask = nullptr; // optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // optional
		multisampling.alphaToOneEnable = VK_FALSE; // optional

		// not setting up depth and stencil testing (VkPipelineDepthStencilState)

		// set up color blending (entirely disabled)
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // optional
		colorBlending.blendConstants[1] = 0.0f; // optional
		colorBlending.blendConstants[2] = 0.0f; // optional
		colorBlending.blendConstants[3] = 0.0f; // optional

		// not setting up dynamic state (VkDynamicState)

		// set up pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // optional

		if (
				vkCreatePipelineLayout(
						logicalDevice_,
						&pipelineLayoutInfo,
						nullptr,
						&pipelineLayout_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}

		// finally, set up the pipeline itself
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // optional
		pipelineInfo.layout = pipelineLayout_;
		pipelineInfo.renderPass = renderPass_;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // optional, not deriving from an existing pipeline
		pipelineInfo.basePipelineIndex = -1; // optional

		if (
				vkCreateGraphicsPipelines(
						logicalDevice_,
						VK_NULL_HANDLE, // optional VkPipelineCache
						1, // number of pipelines to create
						&pipelineInfo,
						nullptr,
						&graphicsPipeline_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline");
		}

		vkDestroyShaderModule(logicalDevice_, fragShaderModule, nullptr);
		vkDestroyShaderModule(logicalDevice_, vertShaderModule, nullptr);
	}

	VkShaderModule createShaderModule(const std::vector<char>& spirVCode) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirVCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(spirVCode.data());

		VkShaderModule shaderModule;

		if (
			vkCreateShaderModule(
				logicalDevice_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}

		return shaderModule;
	}

	// **************************************************************************
	// * Frame Buffers
	// **************************************************************************

	void createFrameBuffers() {
		swapChainFramebuffers_.resize(swapChainImageViews_.size());

		for (size_t i = 0; i < swapChainImageViews_.size(); i++) {
			VkImageView attachments[] = { swapChainImageViews_[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass_;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent_.width;
			framebufferInfo.height = swapChainExtent_.height;
			framebufferInfo.layers = 1;

			if (
					vkCreateFramebuffer(
							logicalDevice_,
							&framebufferInfo,
							nullptr,
							&swapChainFramebuffers_[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer");
			}
		}
	}

	// **************************************************************************
	// * Command Pool
	// **************************************************************************

	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice_);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0; // optional

		if (
				vkCreateCommandPool(
						logicalDevice_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool");
		}
	}

	// **************************************************************************
	// * Vertex Buffer
	// **************************************************************************

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			bool typeMatches = typeFilter & (1 << i);
			bool propertiesMatch =
					(memProperties.memoryTypes[i].propertyFlags & properties) == properties;

			if (typeMatches && propertiesMatch) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type");
	}

	void* vertexData_;

	void createVertexBuffer() {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(vertices[0]) * vertices.size();
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (
				vkCreateBuffer(logicalDevice_, &bufferInfo, nullptr, &vertexBuffer_) !=
				VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(
				logicalDevice_, vertexBuffer_, &memRequirements);

		VkMemoryPropertyFlags desiredProperties =
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(
				memRequirements.memoryTypeBits, desiredProperties);

		if (
				vkAllocateMemory(
						logicalDevice_,
						&allocInfo,
						nullptr,
						&vertexBufferMemory_) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory");
		}

		vkBindBufferMemory(
				logicalDevice_,
				vertexBuffer_,
				vertexBufferMemory_,
				0); // offset within region of memory

		// void* data;
		vkMapMemory(
				logicalDevice_,
				vertexBufferMemory_,
				0, // offset
				bufferInfo.size,
				0, // flags
				&vertexData_);

		// memcpy(data, vertices.data(), (size_t)bufferInfo.size);

		// vkUnmapMemory(logicalDevice_, vertexBufferMemory_);
	}

	// **************************************************************************
	// * Command Buffers
	// **************************************************************************

	void createCommandBuffers() {
		commandBuffers_.resize(swapChainFramebuffers_.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool_;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // submitted to queue directly, can't be called from other command buffers
		allocInfo.commandBufferCount = (uint32_t)commandBuffers_.size();

		if (
				vkAllocateCommandBuffers(
						logicalDevice_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}

		for (size_t i = 0; i < commandBuffers_.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // optional
			beginInfo.pInheritanceInfo = nullptr; // optional, only for secondary command buffers

			if (vkBeginCommandBuffer(commandBuffers_[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass_;
			renderPassInfo.framebuffer = swapChainFramebuffers_[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent_;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(
					commandBuffers_[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(
					commandBuffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);

			VkBuffer vertexBuffers[] = { vertexBuffer_ };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(
					commandBuffers_[i],
					0, // offset
					1, // number of bindings
					vertexBuffers,
					offsets);

			vkCmdDraw(
					commandBuffers_[i],
					static_cast<uint32_t>(vertices.size()), // vertex count
					1, // instance count (not using instanced rendering here)
					0, // first vertex
					0); // first instance

			vkCmdEndRenderPass(commandBuffers_[i]);

			if (vkEndCommandBuffer(commandBuffers_[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer");
			}
		}
	}

	// **************************************************************************
	// * Semaphores and Fences
	// **************************************************************************

	void createSyncObjects() {
		imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);

		inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

		// we don't actually initialize this, it's just a container for
		// each inFlightFences_ being used by the current frame
		imagesInFlight_.resize(swapChainImages_.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // init in the signaled state

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			// create semaphores
			auto imageAvailableSemaphoreCreationResult = vkCreateSemaphore(
					logicalDevice_,
					&semaphoreInfo,
					nullptr,
					&imageAvailableSemaphores_[i]);

			auto renderFinishedSemaphoreCreationResult = vkCreateSemaphore(
					logicalDevice_,
					&semaphoreInfo,
					nullptr,
					&renderFinishedSemaphores_[i]);

			// create fence
			auto fenceCreationResult = vkCreateFence(
					logicalDevice_,
					&fenceInfo,
					nullptr,
					&inFlightFences_[i]);

			if (
				  imageAvailableSemaphoreCreationResult != VK_SUCCESS ||
				  renderFinishedSemaphoreCreationResult != VK_SUCCESS ||
					fenceCreationResult != VK_SUCCESS) {
				throw std::runtime_error("failed to create semaphores for a frame");
			}
		}
	}

	GLFWwindow* window_;

	VkInstance instance_;
	VkDebugUtilsMessengerEXT debugMessenger_;
	VkSurfaceKHR surface_;

	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDevice logicalDevice_;

	VkQueue graphicsQueue_;
	VkQueue presentQueue_;

	VkSwapchainKHR swapChain_;
	std::vector<VkImage> swapChainImages_;
	VkFormat swapChainImageFormat_;
	VkExtent2D swapChainExtent_;
	std::vector<VkImageView> swapChainImageViews_;
	std::vector<VkFramebuffer> swapChainFramebuffers_;

	VkRenderPass renderPass_;
	VkPipelineLayout pipelineLayout_;
	VkPipeline graphicsPipeline_;

	VkCommandPool commandPool_;
	std::vector<VkCommandBuffer> commandBuffers_;

	VkBuffer vertexBuffer_;
	VkDeviceMemory vertexBufferMemory_;

	std::vector<VkSemaphore> imageAvailableSemaphores_;
	std::vector<VkSemaphore> renderFinishedSemaphores_;

	std::vector<VkFence> inFlightFences_;
	std::vector<VkFence> imagesInFlight_;

	size_t currentFrame_ = 0;

	bool framebufferResized_ = false;
	bool shouldRecreateSwapChain_ = false;
};





uint32_t fps = 0;
std::chrono::steady_clock::time_point lastPrintTime;

void logFPS() {
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
	Renderer renderer = Renderer::create();

	// for logFPS
	lastPrintTime = std::chrono::steady_clock::now();

	uint32_t spinFactor = 0;
	double pi = 3.14159265358979323846;

	double one_twenty = 2 * pi / 3;
	double two_forty = 2 * one_twenty;

	try {
		renderer.init();

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

			renderer.draw();
			logFPS();
		}

		renderer.cleanup();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
