#pragma once
#include <glm/glm.hpp>

enum class MaterialType: uint32_t {
	Lambertian = 0,
	Metal = 1
};

struct Sphere {
	glm::vec3 center;
	float radius;
	glm::vec3 albedo;
	MaterialType mat;

	static std::vector<Sphere> generateSpheres(const uint32_t count) {
		std::vector<Sphere> spheres = {
			{
				.center = {-1.0, 0.0, -1.0f},
				.radius = 0.5f,
				.albedo = {0.8, 0.8, 0.8},
				.mat = MaterialType::Metal
			},
			{
				.center = {1.0, 0.0, -1.0},
				.radius = 0.5f,
				.albedo = {0.8, 0.6, 0.2},
				.mat = MaterialType::Metal
			},
			{
				.center = {0.0f, 0.0f, -1.0f},
				.radius = 0.5f,
				.albedo = {0.1, 0.2, 0.5},
				.mat = MaterialType::Lambertian
			},
			{
				.center = {0.0f, -100.5f, -1.0f},
				.radius = 100.0f,
				.albedo = {0.8, 0.8, 0.0},
				.mat = MaterialType::Lambertian
			}
		};

		return spheres;
	}
};
