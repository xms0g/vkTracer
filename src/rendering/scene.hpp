#pragma once
#include <memory>
#include "quad.hpp"
#include "sphere.hpp"
#include "bvh.hpp"

struct SceneData {
	std::shared_ptr<BVHNode> bvh;
	size_t sphereCount;
	size_t quadCount;
	size_t bvhNodeCount;
};

namespace Scene {
SceneData buildScene();

}
