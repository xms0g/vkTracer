#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "aabb.hpp"
#include "hittable.hpp"
#include "texture.hpp"
#include "material.hpp"

struct GPUQuad {
	glm::vec4 Q;
	glm::vec4 u;
	glm::vec4 v;
	glm::vec4 normal;
	glm::vec3 w;
	float D;
	Material mat;
};

struct Quad : Hittable {
	glm::vec3 Q;
	glm::vec3 u;
	glm::vec3 v;
	glm::vec3 normal;
	glm::vec3 w;
	float D;
	Material mat;

	Quad(const glm::vec3 Q, const glm::vec3 u, const glm::vec3 v, const Material& mat)
		: Q(Q), u(u), v(v), mat(mat) {
		const glm::vec3 n = cross(u, v);
		w = n / dot(n, n);
		normal = normalize(n);
		D = dot(normal, Q);
	}

	~Quad() override = default;

	[[nodiscard]]
	AABB boundingBox() const override {
		const auto diagonal1 = AABB(Q, Q + u + v);
		const auto diagonal2 = AABB(Q + u, Q + v);
		return {diagonal1, diagonal2};
	}

	static std::vector<std::shared_ptr<Hittable> > generateQuads() {
		std::vector<std::shared_ptr<Hittable> > quads;

		quads.emplace_back(std::make_shared<Quad>(
			glm::vec3(0, 3, 0),
			glm::vec3(0, 0, -4),
			glm::vec3(0, 4, 0),
			Material{
				.texture = {.type = TextureType::SolidColor, .color = glm::vec3(1.0, 0.2, 0.2)},
				.type = MaterialType::Lambertian
			}));

		return quads;
	}
};
