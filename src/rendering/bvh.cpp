#include "bvh.hpp"
#include <algorithm>
#include <cstdint>
#include <random>

size_t BVHNode::count = 0;

BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable> >& spheres, const size_t start, const size_t end) {
	++count;

	for (size_t object_index = start; object_index < end; object_index++)
		bounds = AABB(bounds, spheres[object_index]->boundingBox());

	const int axis = bounds.longestAxis();

	if (const size_t object_span = end - start; object_span == 1) {
		left = right = spheres[start];
	} else if (object_span == 2) {
		left = spheres[start];
		right = spheres[start + 1];
	} else {
		std::sort(std::begin(spheres) + start, std::begin(spheres) + end,
		          [&](const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
			          const auto a_axis_interval = a->boundingBox().axis(axis);
			          const auto b_axis_interval = b->boundingBox().axis(axis);
			          return a_axis_interval.x < b_axis_interval.x;
		          });

		const auto mid = start + object_span / 2;
		left = std::make_shared<BVHNode>(spheres, start, mid);
		right = std::make_shared<BVHNode>(spheres, mid, end);
	}

	bounds = AABB(left->boundingBox(), right->boundingBox());
}

AABB BVHNode::boundingBox() const {
	return bounds;
}

std::vector<GPUBVHNode> BVHNode::flatten(const BVHNode& root,
                                         std::vector<GPUSphere>& gpuSpheres,
                                         const uint64_t bvhBaseAddress,
                                         const uint64_t sphereBaseAddress) {
	std::vector<GPUBVHNode> gpuNodes;
	std::unordered_map<const Sphere*, uint64_t> sphereAdresses;

	auto getSphereAddress = [&](const Sphere* sphere) -> uint64_t {
		if (const auto it = sphereAdresses.find(sphere); it != sphereAdresses.end()) {
			return it->second;
		}

		const auto index = static_cast<uint32_t>(gpuSpheres.size());
		const uint64_t address = sphereBaseAddress + index * sizeof(GPUSphere);
		sphereAdresses[sphere] = address;

		gpuSpheres.emplace_back(sphere->centerRadius, sphere->center2, sphere->mat);

		return address;
	};


	std::function<uint32_t(const BVHNode&)> visit = [&](const BVHNode& node) -> uint32_t {
		const uint32_t index = gpuNodes.size();

		// Reserve the slot before processing children.
		gpuNodes.emplace_back();

		gpuNodes[index].bounds = node.bounds;

		if (const auto* bvh = dynamic_cast<BVHNode*>(node.left.get())) {
			const uint32_t leftIndex = visit(*bvh);
			gpuNodes[index].left = {
				.address = bvhBaseAddress + leftIndex * sizeof(GPUBVHNode),
				.type = BVHType
			};
		} else if (const auto* sphere = dynamic_cast<Sphere*>(node.left.get())) {
			gpuNodes[index].left = {.address = getSphereAddress(sphere), .type = SphereType};
		}

		if (const auto* bvh = dynamic_cast<BVHNode*>(node.right.get())) {
			const uint32_t rightIndex = visit(*bvh);
			gpuNodes[index].right = {
				.address = bvhBaseAddress + rightIndex * sizeof(GPUBVHNode),
				.type = BVHType
			};
		} else if (const auto* sphere = dynamic_cast<Sphere*>(node.right.get())) {
			gpuNodes[index].right = {.address = getSphereAddress(sphere), .type = SphereType};
		}

		return index;
	};

	visit(root);

	return gpuNodes;
}
