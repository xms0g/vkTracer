#include "scene.hpp"
#include "quad.hpp"
#include "sphere.hpp"
#include "bvh.hpp"
#include "volume.hpp"

SceneData Scene::buildScene() {
	std::vector<std::shared_ptr<Hittable> > cornellBox;
	std::vector<std::shared_ptr<Hittable> > spheres;
	std::vector<std::shared_ptr<Hittable> > volumes;

	// Right
	cornellBox.emplace_back(std::make_shared<Quad>(
		glm::vec3(1.5f, -1.5f, -5.0f),
		glm::vec3(0.0f, 3.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 3.0f),
		Material{
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(0.12f, 0.45f, 0.15f)},
			.type = MaterialType::Lambertian,
		}));
	// Left
	cornellBox.emplace_back(std::make_shared<Quad>(
		glm::vec3(-1.5f, -1.5f, -5.0f),
		glm::vec3(0.0f, 3.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 3.0f),
		Material{
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(0.65f, 0.05f, 0.05f)},
			.type = MaterialType::Lambertian,
		}));
	// Floor
	cornellBox.emplace_back(std::make_shared<Quad>(
		glm::vec3(-1.5f, -1.5f, -5.0f),
		glm::vec3(3.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 3.0f),
		Material{
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(0.73f)},
			.type = MaterialType::Lambertian,
		}));
	// Ceiling
	cornellBox.emplace_back(std::make_shared<Quad>(
		glm::vec3(1.5f, 1.5f, -2.0f),
		glm::vec3(-3.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -3.0f),
		Material{
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(0.73f)},
			.type = MaterialType::Lambertian,
		}));
	// Back
	cornellBox.emplace_back(std::make_shared<Quad>(
		glm::vec3(-1.5f, -1.5f, -2.0f),
		glm::vec3(3.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 3.0f, 0.0f),
		Material{
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(0.73f)},
			.type = MaterialType::Lambertian,
		}));
	// Light
	cornellBox.emplace_back(std::make_shared<Quad>(
		glm::vec3(0.75f, 1.49f, -2.75f),
		glm::vec3(-1.5f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		Material{
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(4.0f)},
			.type = MaterialType::DiffuseLight,
		}));

	spheres.emplace_back(std::make_shared<Sphere>(
		glm::vec3(0.8, -1.0, -3.0),
		glm::vec3(0.0, 0.0, 0.0f),
		0.5f,
		Material{
			.texture = {.type = TextureType::Noise, .noise = {7.0, 0.5, 2.0, 0.0}},
			.type = MaterialType::Lambertian
		}));

	spheres.emplace_back(std::make_shared<Sphere>(
		glm::vec3(-0.8, -1.0, -3.0),
		glm::vec3(0.0, 0.0, 0.0f),
		0.5f,
		Material{
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(0.7, 0.6, 0.5)},
			.type = MaterialType::Metal,
		}));

	spheres.emplace_back(std::make_shared<Sphere>(
		glm::vec3(0.0, -1.0, -4.0),
		glm::vec3(0.0, 0.0, 0.0f),
		0.5f,
		Material{
			.type = MaterialType::Dielectric,
			.refractionIndex = 1.5f
		}));

	// const auto box = Quad::box(
	// 	glm::vec3(0.0f, -1.5f, -5.0f),
	// 	glm::vec3(1.0f, 0.5f, -4.0f),
	// 	0.0f,
	// 	Material{
	// 		.texture = {.type = TextureType::SolidColor, .color = glm::vec3(0.73, 0.73, 0.73)},
	// 		.type = MaterialType::Lambertian
	// 	});

	volumes.emplace_back(std::make_shared<Volume>(
		spheres[0],
		1.0f,
		Material{
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(0.0, 0.0, 0.0)},
			.type = MaterialType::Isotropic
		}));

	std::vector<std::shared_ptr<Hittable> > hittables = spheres;
	hittables.insert(hittables.end(), cornellBox.begin(), cornellBox.end());
	hittables.insert(hittables.end(), volumes.begin(), volumes.end());
	//hittables.insert(hittables.end(), box.begin(), box.end());

	return {
		.bvh = std::make_shared<BVHNode>(hittables, 0, hittables.size()),
		.sphereCount = spheres.size(),
		.quadCount = cornellBox.size(),
		.volumeCount = volumes.size(),
		.bvhNodeCount = BVHNode::count,
	};
}
