#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "input.h"

#include <chrono>
#include <iostream>


constexpr double MICROSECONDS = 1000000.0;
constexpr double MAX_SPEED = 1.0f / MICROSECONDS;

void logvec3(glm::vec3 vec) {
	std::cout << "x: " << vec.x << ", y: " << vec.y << ", z: " << vec.z << std::endl;
}

struct Camera {
	glm::vec3 position = glm::vec3(2.0f, 2.0f, 2.0f);
	glm::vec3 direction = glm::vec3(-2.0f, -2.0f, -2.0f);
	const glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
	// values in degrees
	double yaw = 225.0f;
	double pitch = -35.26f;

	void update(Input::KeyStates keys, Input::MouseState mouse, std::chrono::microseconds frameDuration) {
		// update direction based on mouse state
		yaw -= mouse.xOffset;
		pitch -= mouse.yOffset;

		// clamp pitch within 89 degrees
		if (pitch > 89.0f) { pitch = 89.0f; }
		if (pitch < -89.0f) { pitch = -89.0f; }

		direction = getDirection(yaw, pitch);

		float speed = MAX_SPEED * frameDuration.count();

		// update position based on key states
		if (keys.forward) { position += speed * direction; }
		if (keys.reverse) { position -= speed * direction; }
		if (keys.left) { position -= glm::normalize(glm::cross(direction, up)) * speed; }
		if (keys.right) { position += glm::normalize(glm::cross(direction, up)) * speed; }
		if (keys.rise) { position.z += speed; }
		if (keys.fall) { position.z -= speed; }
	}

	static glm::vec3 getDirection(double yaw, double pitch) {
		double x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		double z = sin(glm::radians(pitch));
		double y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		return glm::vec3(x, y, z);
	}
};
