#pragma once

enum HittableType : uint32_t {
	SphereType,
	BVHType
};

struct HittableRef {
	uint64_t address;
	HittableType type;
};

struct Hittable {
	virtual ~Hittable() = default;

	[[nodiscard]]
	virtual AABB boundingBox() const = 0;
};
