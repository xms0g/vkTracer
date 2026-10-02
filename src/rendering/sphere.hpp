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
	glm::vec3 center;
	glm::vec3 center2;
	float radius;
	Material mat;

	Sphere(const glm::vec3 center, const glm::vec3 center2, const float radius, const Material& mat)
		: center(glm::vec3(center)), center2(center2), radius(radius), mat(mat) {
	}

	~Sphere() override = default;

	[[nodiscard]]
	AABB boundingBox() const override {
		return AABB{
			glm::vec2(center.x - radius, center.x + radius),
			glm::vec2(center.y - radius, center.y + radius),
			glm::vec2(center.z - radius, center.z + radius)
		};
	}
};
