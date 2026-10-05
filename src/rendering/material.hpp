#pragma once
#include "texture.hpp"

enum class MaterialType: uint32_t {
	Lambertian = 0,
	Metal,
	Dielectric,
	DiffuseLight,
	Isotropic
};

struct alignas(16) Material {
	Texture texture;
	MaterialType type;
	float fuzz;
	float refractionIndex;
	uint32_t padding;
};
