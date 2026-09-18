#pragma once
#include <glm/glm.hpp>

enum class MaterialType: uint32_t {
	Lambertian = 0,
	Metal = 1
};

struct Material {
	glm::vec4 albedo;
	MaterialType type;
	float fuzz;
	uint32_t pad[2];
};

struct Sphere {
	glm::vec4 centerRadius;
	Material mat;

	static std::vector<Sphere> generateSpheres() {
		std::vector<Sphere> spheres = {
			{
				.centerRadius = {-1.0, 0.0, -1.0f, 0.5f},
				.mat = {
					.albedo = {0.8, 0.8, 0.8, 0.0},
					.type = MaterialType::Metal,
					.fuzz = 0.3f
				}
			},
			{
				.centerRadius = {1.0, 0.0, -1.0, 0.5f},
				.mat = {
					.albedo = {0.8, 0.6, 0.2, 0.0},
					.type = MaterialType::Metal,
					.fuzz = 0.0f
				}
			},
			{
				.centerRadius = {0.0f, 0.0f, -1.0f, 0.5f},
				.mat = {
					.albedo = {0.1, 0.2, 0.5, 0.0},
					.type = MaterialType::Lambertian
				}
			},
			{
				.centerRadius = {0.0f, -100.5f, -1.0f, 100.0f},
				.mat = {
					.albedo = {0.8, 0.8, 0.0, 0.0},
					.type = MaterialType::Lambertian
				}
			}
		};

		return spheres;
	}
};
