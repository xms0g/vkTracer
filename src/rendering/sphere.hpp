#pragma once
#include <glm/glm.hpp>
#include "hittable.hpp"
#include "aabb.hpp"
#include "material.hpp"

struct GPUSphere {
	glm::vec4 centerRadius;
	glm::vec4 center2;
	Material mat;
};

struct Sphere : Hittable {
	glm::vec4 centerRadius;
	glm::vec4 center2;
	Material mat;

	Sphere(const glm::vec4 centerRadius, const glm::vec4 center2, const Material mat)
		: centerRadius(centerRadius), center2(center2), mat(mat) {
	}

	~Sphere() override = default;

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
};
