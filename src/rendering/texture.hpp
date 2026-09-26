#pragma once
#include <glm/glm.hpp>

enum class TextureType: uint32_t {
	SolidColor,
	Checker,
	Image
};

struct Texture {
	TextureType type;
	glm::vec3 color;
};
