#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <array>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	// bindings describe the spacing between data and whether the data is per-
	// vertex or per-instance
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0; // all vertex data is in one array, so we only have one binding
		bindingDescription.stride = sizeof(Vertex); // number of bytes between entries
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // move to the next data entry after each vertex (not using instanced rendering)

		return bindingDescription;
	}

	// attributes describe the type of attributes passed to the vertex shader,
	// which binding to load the from, and at which offset
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		// position is specified as a vec3 of signed floats
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		// so is color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		// texture coordinate is a vec2 of signed floats
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		// ivec2: VK_FORMAT_R32G32_SINT, a 2-component vector of 32-bit signed integers
		// uvec4 : VK_FORMAT_R32G32B32A32_UINT, a 4-component vector of 32-bit unsigned integers
		// double : VK_FORMAT_R64_SFLOAT, a double-precision (64-bit) float

		return attributeDescriptions;
	}
};

