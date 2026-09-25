#pragma once
#include <random>
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

struct AABB {
	glm::vec2 x{};
	glm::vec2 y{};
	glm::vec2 z{};

	AABB() = default;

	AABB(const glm::vec2 x, const glm::vec2 y, const glm::vec2 z)
		: x(x), y(y), z(z) {}

	AABB(const AABB a, const AABB b) {
		x = glm::vec2(fmin(a.x.x, b.x.x), std::fmax(a.x.y, b.x.y));
		y = glm::vec2(fmin(a.y.x, b.y.x), std::fmax(a.y.y, b.y.y));
		z = glm::vec2(fmin(a.z.x, b.z.x), std::fmax(a.z.y, b.z.y));
	}

	[[nodiscard]]
	glm::vec2 axis(const int axis) const {
		if (axis == 0) return x;
		if (axis == 1) return y;
		return z;
	}

	[[nodiscard]]
	int32_t longestAxis() const {
		// Returns the index of the longest axis of the bounding box.
		const float xSize = x.t - x.s;
		const float ySize = y.t - y.s;
		const float zSize = z.t - z.s;

		if (xSize > ySize)
			return xSize > zSize ? 0 : 2;

		return ySize > zSize ? 1 : 2;
	}
};

struct Hittable {
	virtual ~Hittable() = default;

	[[nodiscard]]
	virtual AABB boundingBox() const = 0;
};

struct GPUSphere {
	glm::vec4 centerRadius;
	glm::vec4 center2;
	Material mat;
};

struct Sphere : Hittable {
	glm::vec4 centerRadius;
	glm::vec4 center2;
	Material mat;

	Sphere(glm::vec4 centerRadius, glm::vec4 center2, Material mat)
		: centerRadius(centerRadius), center2(center2), mat(mat) {
	}

	[[nodiscard]]
	AABB boundingBox() const override {
		const auto center = glm::vec3(centerRadius);
		const float radius = centerRadius.w;

		return AABB{
			glm::vec2(center.x - radius, center.x + radius),
			glm::vec2(center.y - radius, center.y + radius),
			glm::vec2(center.z - radius, center.z + radius)
		};
	}

	static std::vector<std::shared_ptr<Hittable>> generateSpheres() {
		std::vector<std::shared_ptr<Hittable>> spheres;

		spheres.emplace_back(std::make_shared<Sphere>(
			glm::vec4(0.0, 1.0, 0.0f, 1.0f),
			glm::vec4(0.0, 0.0, 0.0f, 0.0f), Material{
				.type = MaterialType::Dielectric,
				.refractionIndex = 1.5
			}));
		spheres.emplace_back(std::make_shared<Sphere>(
			glm::vec4(-4.0, 1.0, 0.0, 1.0f),
			glm::vec4(0.0, 0.0, 0.0f, 0.0f), Material{
				.albedo = {0.4, 0.2, 0.1, 0.0},
				.type = MaterialType::Lambertian
			}));

		spheres.emplace_back(std::make_shared<Sphere>(
			glm::vec4(4.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0, 0.0, 0.0f, 0.0f), Material{
				.albedo = {0.7, 0.6, 0.5, 0.0},
				.type = MaterialType::Metal,
			}));

		spheres.emplace_back(std::make_shared<Sphere>(
			glm::vec4(0.0f, -100.5f, 0.0f, 100.5f),
			glm::vec4(0.0, 0.0, 0.0f, 0.0f), Material{
				.albedo = {0.8, 0.8, 0.5, 0.0},
				.type = MaterialType::Lambertian
			}));

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
						auto center2 = glm::vec4(center + glm::vec3(0, fuzzDis(gen), 0), 0.0);
						spheres.emplace_back(std::make_shared<Sphere>(centerRad, center2, mat));
					} else if (chooseMat < 0.95) {
						// metal
						const auto albedo = glm::vec4(albedoDis(gen), albedoDis(gen), albedoDis(gen), 0.0f);
						Material mat = {.albedo = albedo, .type = MaterialType::Metal, .fuzz = fuzzDis(gen)};
						spheres.emplace_back(std::make_shared<Sphere>(centerRad, glm::vec4(0.0), mat));
					} else {
						// glass
						Material mat = {.type = MaterialType::Dielectric, .refractionIndex = 1.5f};
						spheres.emplace_back(std::make_shared<Sphere>(centerRad, glm::vec4(0.0), mat));
					}
				}
			}
		}

		return spheres;
	}
};
