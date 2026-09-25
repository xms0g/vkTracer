#pragma once
#include <glm/glm.hpp>
#include "aabb.hpp"
#include "hittable.hpp"

struct GPUSphere;

struct GPUBVHNode {
	AABB bounds;
	HittableRef left{};
	HittableRef right{};
};

struct BVHNode : Hittable {
	AABB bounds;
	std::shared_ptr<Hittable> left{};
	std::shared_ptr<Hittable> right{};
	static size_t count;

	BVHNode() = default;

	BVHNode(std::vector<std::shared_ptr<Hittable> >& objects, size_t start, size_t end);

	[[nodiscard]]
	AABB boundingBox() const override;

	static std::vector<GPUBVHNode> flatten(const BVHNode& root,
	                                       std::vector<GPUSphere>& gpuSpheres,
	                                       uint64_t bvhBaseAddress,
	                                       uint64_t sphereBaseAddress);
};
