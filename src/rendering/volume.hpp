#pragma once
#include "aabb.hpp"
#include "hittable.hpp"
#include "material.hpp"

struct GPUVolume {
	glm::vec4 negInvDensity;
	Material mat;
	HittableRef boundary;
};

struct Volume : Hittable {
	std::shared_ptr<Hittable> boundary;
	float negInvDensity;
	Material mat;

	Volume(const std::shared_ptr<Hittable>& boundary, const float density, const Material& mat)
		: boundary(boundary), negInvDensity(-1.0f / density), mat(mat) {
	}

	[[nodiscard]]
	AABB boundingBox() const override {
		return boundary->boundingBox();
	}
};
