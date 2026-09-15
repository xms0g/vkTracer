#pragma once
#include <glm/glm.hpp>

struct Sphere {
	glm::vec3 center;
	float radius;

	static std::vector<Sphere> generateSpheres(const uint32_t count) {
		std::vector<Sphere> spheres = {
			{.center = { 0.0f,  0.0f, -1.0f }, .radius = 0.5f},
			{.center = { 0.0f, -100.5f, -1.0f }, .radius = 100.0f}
		};

		return spheres;
	}
};
