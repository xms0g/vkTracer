#pragma once
#include <array>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include "vertex.hpp"

struct Particle {
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec4 color;

	static VertexLayout layout() {
		return {
			{0, sizeof(Particle), vk::VertexInputRate::eVertex},
			{
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Particle, position)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Particle, color)),
			}
		};
	}

	static std::vector<Particle> generate(const uint32_t count, const float width, const float height) {
		// Initialize particles
		std::default_random_engine rndEngine(static_cast<unsigned>(time(nullptr)));
		std::uniform_real_distribution rndDist(0.0f, 1.0f);

		// Initial particle positions on a circle
		std::vector<Particle> particles(count);
		for (auto& [position, velocity, color]: particles) {
			const float r = 0.25f * sqrtf(rndDist(rndEngine));
			const float theta = rndDist(rndEngine) * 2.0f * 3.14159265358979323846f;
			const float x = r * cosf(theta) * height / width;
			const float y = r * sinf(theta);
			position = glm::vec2(x, y);
			velocity = normalize(glm::vec2(x, y)) * 0.5f;
			color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
		}

		return particles;
	}
};
