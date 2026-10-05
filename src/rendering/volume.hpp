#pragma once
#include "aabb.hpp"
#include "hittable.hpp"
#include "material.hpp"

struct alignas(16) GPUVolume {
	Material mat;
	HittableRef boundary;
	float negInvDensity;
	uint32_t padding;
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
