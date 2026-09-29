#pragma once

enum class MaterialType: uint32_t {
	Lambertian = 0,
	Metal,
	Dielectric,
	DiffuseLight,
};

struct Material {
	Texture texture;
	MaterialType type;
	float fuzz;
	float refractionIndex;
	uint32_t pad;
};
