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
	glm::vec3 normal{};
	glm::vec3 w{};
	float D;
	Material mat;

	Quad(const glm::vec3& Q, const glm::vec3& u, const glm::vec3& v, const Material& mat)
		: Q(Q), u(u), v(v), mat(mat) {
		const glm::vec3 n = glm::cross(u, v);
		w = n / glm::dot(n, n);
		normal = glm::normalize(n);
		D = glm::dot(normal, Q);
	}

	~Quad() override = default;

	[[nodiscard]]
	AABB boundingBox() const override {
		const auto diagonal1 = AABB(Q, Q + u + v);
		const auto diagonal2 = AABB(Q + u, Q + v);
		return {diagonal1, diagonal2};
	}

	static std::vector<std::shared_ptr<Hittable> > box(const glm::vec3& a,
	                                                   const glm::vec3& b,
	                                                   float angle,
	                                                   Material mat) {
		std::vector<std::shared_ptr<Hittable> > sides;

		auto min = glm::vec3(std::fmin(a.x, b.x), std::fmin(a.y, b.y), std::fmin(a.z, b.z));
		auto max = glm::vec3(std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z));

		glm::vec3 center = 0.5f * (min + max);

		auto dx = glm::vec3(max.x - min.x, 0, 0);
		auto dy = glm::vec3(0, max.y - min.y, 0);
		auto dz = glm::vec3(0, 0, max.z - min.z);

		auto addQuad = [&](glm::vec3 Q,
		                   glm::vec3 u,
		                   glm::vec3 v) {
			Q = rotateY(Q, center, angle);
			u = rotateYVector(u, angle);
			v = rotateYVector(v, angle);

			sides.emplace_back(
				std::make_shared<Quad>(Q, u, v, mat)
			);
		};

		addQuad(glm::vec3(min.x, min.y, max.z), dx, dy); // front
		addQuad(glm::vec3(max.x, min.y, max.z), -dz, dy); // right
		addQuad(glm::vec3(max.x, min.y, min.z), -dx, dy); // back
		addQuad(glm::vec3(min.x, min.y, min.z), dz, dy); // left
		addQuad(glm::vec3(min.x, max.y, max.z), dx, -dz); // top
		addQuad(glm::vec3(min.x, min.y, min.z), dx, dz); // bottom

		return sides;
	}

	static glm::vec3 rotateY(const glm::vec3& p,
	                         const glm::vec3& center,
	                         const float angle) {
		const float radians = glm::radians(angle);
		const float c = std::cos(radians);
		const float s = std::sin(radians);

		const glm::vec3 q = p - center;

		return center + glm::vec3(
			       c * q.x + s * q.z,
			       q.y,
			       -s * q.x + c * q.z
		       );
	}

	static glm::vec3 rotateYVector(const glm::vec3& v, const float angle) {
		const float radians = glm::radians(angle);
		const float c = std::cos(radians);
		const float s = std::sin(radians);

		return {
			c * v.x + s * v.z,
			v.y,
			-s * v.x + c * v.z
		};
	}
};
