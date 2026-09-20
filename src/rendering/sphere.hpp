#pragma once
#include <glm/glm.hpp>

enum class MaterialType: uint32_t {
	Lambertian = 0,
	Metal = 1,
	Dielectric = 2
};

struct Material {
	glm::vec4 albedo;
	MaterialType type;
	float fuzz;
	float refractionIndex;
	uint32_t pad;
};

struct Sphere {
	glm::vec4 centerRadius;
	glm::vec4 center2;
	Material mat;

	static std::vector<Sphere> generateSpheres() {
		std::vector<Sphere> spheres = {
			{
				.centerRadius = {0.0, 1.0, 0.0f, 1.0f},
				.mat = {
					.type = MaterialType::Dielectric,
					.refractionIndex = 1.5
				}
			},
			{
				.centerRadius = {-4.0, 1.0, 0.0, 1.0f},
				.mat = {
					.albedo = {0.4, 0.2, 0.1, 0.0},
					.type = MaterialType::Lambertian,
				}
			},
			{
				.centerRadius = {4.0f, 1.0f, 0.0f, 1.0f},
				.mat = {
					.albedo = {0.7, 0.6, 0.5, 0.0},
					.type = MaterialType::Metal,
				}
			},
			{
				.centerRadius = {0.0f, -100.5f, 0.0f, 100.5f},
				.mat = {
					.albedo = {0.8, 0.8, 0.5, 0.0},
					.type = MaterialType::Lambertian
				}
			},
		};

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis;
		std::uniform_real_distribution<float> albedoDis(0.5, 1);
		std::uniform_real_distribution<float> fuzzDis(0, 0.5);

		for (int a = -4; a < 4; a++) {
			for (int b = -4; b < 4; b++) {
				const auto chooseMat = dis(gen);
				glm::vec3 center(a + 0.9 * dis(gen), 0.2, b + 0.9 * dis(gen));
				auto centerRad = glm::vec4(center, 0.2);

				if (glm::length(center - glm::vec3(4, 0.2, 0)) > 0.9) {
					if (chooseMat < 0.8) {
						// diffuse
						const auto albedo = glm::vec4(dis(gen), dis(gen), dis(gen), 0.0f);
						Material mat = {.albedo = albedo, .type = MaterialType::Lambertian};
						auto center2 = glm::vec4(center + glm::vec3(0, fuzzDis(gen), 0), 0);
						spheres.emplace_back(centerRad, center2, mat);
					} else if (chooseMat < 0.95) {
						// metal
						const auto albedo = glm::vec4(albedoDis(gen), albedoDis(gen), albedoDis(gen), 0.0f);
						Material mat = {.albedo = albedo, .type = MaterialType::Metal, .fuzz = fuzzDis(gen)};
						spheres.emplace_back(centerRad, glm::vec4(0.0), mat);
					} else {
						// glass
						Material mat = {.type = MaterialType::Dielectric, .refractionIndex = 1.5f};
						spheres.emplace_back(centerRad, glm::vec4(0.0), mat);
					}
				}
			}
		}

		return spheres;
	}
};
