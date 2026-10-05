#pragma once
#include <glm/glm.hpp>

enum class TextureType: uint32_t {
	SolidColor,
	Checker,
	Noise,
	Image
};

struct alignas(16) Texture {
	glm::vec4 noise;
	glm::vec3 color;
	TextureType type;
};
