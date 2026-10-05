#pragma once

enum HittableType : uint32_t {
	SphereType,
	QuadType,
	VolumeType,
	BVHType
};

struct alignas(16) HittableRef {
	uint64_t address;
	HittableType type;
	uint32_t padding;
};

struct Hittable {
	virtual ~Hittable() = default;

	[[nodiscard]]
	virtual AABB boundingBox() const = 0;
};
