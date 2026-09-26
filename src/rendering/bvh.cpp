#include "bvh.hpp"
#include <algorithm>
#include <cstdint>
#include <random>
#include "sphere.hpp"

size_t BVHNode::count = 0;

BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable> >& objects, const size_t start, const size_t end) {
	++count;

	for (size_t objectIndex = start; objectIndex < end; objectIndex++)
		bounds = AABB(bounds, objects[objectIndex]->boundingBox());

	const int axis = bounds.longestAxis();

	if (const size_t objectCount = end - start; objectCount == 1) {
		left = right = objects[start];
	} else if (objectCount == 2) {
		left = objects[start];
		right = objects[start + 1];
	} else {
		std::sort(std::begin(objects) + start, std::begin(objects) + end,
		          [&](const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
			          const auto a_axis_interval = a->boundingBox().axis(axis);
			          const auto b_axis_interval = b->boundingBox().axis(axis);
			          return a_axis_interval.x < b_axis_interval.x;
		          });

		const auto mid = start + objectCount / 2;
		left = std::make_shared<BVHNode>(objects, start, mid);
		right = std::make_shared<BVHNode>(objects, mid, end);
	}
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

	std::function<uint32_t(const BVHNode&)> visit;

	auto makeRef = [&](const std::shared_ptr<Hittable>& obj) -> HittableRef {
		if (const auto* bvh = dynamic_cast<BVHNode*>(obj.get())) {
			const uint32_t index = visit(*bvh);

			return {
				.address = bvhBaseAddress + index * sizeof(GPUBVHNode),
				.type = BVHType
			};
		}

		if (const auto* sphere = dynamic_cast<Sphere*>(obj.get())) {
			return {.address = getSphereAddress(sphere), .type = SphereType};
		}
		return {};
	};

	visit = [&](const BVHNode& node) -> uint32_t {
		const uint32_t index = gpuNodes.size();

		// Reserve the slot before processing children.
		gpuNodes.emplace_back();

		gpuNodes[index].bounds = node.bounds;
		gpuNodes[index].left = makeRef(node.left);
		gpuNodes[index].right = makeRef(node.right);

		return index;
	};

	visit(root);

	return gpuNodes;
}
