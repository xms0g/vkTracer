#pragma once

enum class MaterialType: uint32_t {
	Lambertian = 0,
	Metal = 1,
	Dielectric = 2
};

struct Material {
	Texture texture;
	MaterialType type;
	float fuzz;
	float refractionIndex;
	uint32_t pad;
};
