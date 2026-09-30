#include "scene.hpp"

SceneData Scene::buildScene() {
	std::vector<std::shared_ptr<Hittable>> spheres;

	// spheres.emplace_back(std::make_shared<Sphere>(
	// 	glm::vec4(0.0, 1.0, 0.0f, 1.0f),
	// 	glm::vec4(0.0, 0.0, 0.0f, 0.0f),
	// 	Material{
	// 		.type = MaterialType::Dielectric,
	// 		.refractionIndex = 1.5
	// 	}));
	spheres.emplace_back(std::make_shared<Sphere>(
		glm::vec4(0.0, 2.0, 0.0, 2.0f),
		glm::vec4(0.0, 0.0, 0.0f, 0.0f),
		Material{
			.texture = {.type = TextureType::Noise, .noise = {7.0, 0.5, 2.0, 0.0}},
			.type = MaterialType::Lambertian
		}));

	// spheres.emplace_back(std::make_shared<Sphere>(
	// 	glm::vec4(3.0f, 1.0f, 0.0f, 1.0f),
	// 	glm::vec4(0.0, 0.0, 0.0f, 0.0f),
	// 	Material{
	// 		.texture = {.type = TextureType::SolidColor, .color = glm::vec3(0.7, 0.6, 0.5)},
	// 		.type = MaterialType::Metal,
	// 	}));

	// spheres.emplace_back(std::make_shared<Sphere>(
	// 	glm::vec4(0.0f, -100.5f, 0.0f, 100.5f),
	// 	glm::vec4(0.0, 0.0, 0.0f, 0.0f),
	// 	Material{
	// 		.texture = {.type = TextureType::Noise, .noise = {7.0, 0.5, 2.0, 0.0}},
	// 		.type = MaterialType::Lambertian
	// 	}));
	const auto light = std::make_shared<Quad>(
		glm::vec3(3, 4, 2),
		glm::vec3(-2, 0, 2),
		glm::vec3(0, 2, -1),
		Material{
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(4, 4, 4)},
			.type = MaterialType::DiffuseLight
		});

	const auto box = Quad::box(
		glm::vec3(-1.5f, -1.5f, -5.0f),
		glm::vec3(1.5f, 1.5f, -2.0f),
		Material{
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(0.73, 0.73, 0.73)},
			.type = MaterialType::Lambertian
		});

	std::vector<std::shared_ptr<Hittable> > hittables = spheres;
	hittables.insert(hittables.end(), box.begin(), box.end());
	hittables.push_back(light);

	return {
		.bvh = std::make_shared<BVHNode>(hittables, 0, hittables.size()),
		.sphereCount = spheres.size(),
		.quadCount = box.size() + 1,
		.bvhNodeCount = BVHNode::count,
	};
}
