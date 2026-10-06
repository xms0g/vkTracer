#pragma once
#include <glm/glm.hpp>

struct AABB {
	glm::vec2 x{};
	glm::vec2 y{};
	glm::vec2 z{};
	uint64_t padding{};

	AABB() = default;

	AABB(const glm::vec2 x, const glm::vec2 y, const glm::vec2 z)
		: x(x), y(y), z(z) {
		padToMin();
	}

	AABB(const glm::vec3& a, const glm::vec3& b) {
		x = glm::vec2(std::fmin(a[0],b[0]), std::fmax(a[0],b[0]));
		y = glm::vec2(std::fmin(a[1],b[1]), std::fmax(a[1],b[1]));
		z = glm::vec2(std::fmin(a[2],b[2]), std::fmax(a[2],b[2]));

		padToMin();
	}

	AABB(const AABB a, const AABB b) {
		x = glm::vec2(fmin(a.x.x, b.x.x), std::fmax(a.x.y, b.x.y));
		y = glm::vec2(fmin(a.y.x, b.y.x), std::fmax(a.y.y, b.y.y));
		z = glm::vec2(fmin(a.z.x, b.z.x), std::fmax(a.z.y, b.z.y));
	}

	[[nodiscard]]
	glm::vec2 axis(const int axis) const {
		if (axis == 0) return x;
		if (axis == 1) return y;
		return z;
	}

	[[nodiscard]]
	int32_t longestAxis() const {
		// Returns the index of the longest axis of the bounding box.
		const float xSize = x.t - x.s;
		const float ySize = y.t - y.s;
		const float zSize = z.t - z.s;

		if (xSize > ySize)
			return xSize > zSize ? 0 : 2;

		return ySize > zSize ? 1 : 2;
	}

	void padToMin() {
		constexpr float delta = 0.0001;
		const float xSize = x.t - x.s;
		const float ySize = y.t - y.s;
		const float zSize = z.t - z.s;

		x = xSize < delta ? glm::vec2(x.s - delta/2, x.t + delta/2) : x;
		y = ySize < delta ? glm::vec2(y.s - delta/2, y.t + delta/2) : y;
		z = zSize < delta ? glm::vec2(z.s - delta/2, z.t + delta/2) : z;
	}
};
