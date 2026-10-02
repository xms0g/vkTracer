#include "scene.hpp"

#include <random>

#include "quad.hpp"
#include "sphere.hpp"
#include "bvh.hpp"
#include "volume.hpp"

SceneData Scene::buildScene() {
	std::vector<std::shared_ptr<Hittable> > cornellBox;

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
			.texture = {.type = TextureType::Checker},
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
		glm::vec3(-1.5f, -1.5f, -5.0f),
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
			.texture = {.type = TextureType::SolidColor, .color = glm::vec3(7.0f)},
			.type = MaterialType::DiffuseLight,
		}));

	std::vector<std::shared_ptr<Hittable> > spheres;

	// Small moving diffuse sphere
	auto center1 = glm::vec3(-0.75f, 0.9f, -3.25f);
	auto center2 = center1 + glm::vec3(0.4f, 0.0f, 0.0f);

	spheres.emplace_back(std::make_shared<Sphere>(
		center1,
		center2,
		0.45f,
		Material{
			.texture = {
				.type = TextureType::SolidColor,
				.color = glm::vec3(0.7f, 0.3f, 0.1f)
			},
			.type = MaterialType::Lambertian
		}));

	// Large metal sphere
	spheres.emplace_back(std::make_shared<Sphere>(
		glm::vec3(0.65f, -0.45f, -3.65f),
		glm::vec3(0.0f),
		0.7f,
		Material{
			.texture = {
				.type = TextureType::SolidColor,
				.color = glm::vec3(0.8f, 0.8f, 0.9f)
			},
			.type = MaterialType::Metal,
		}));

	// Image-textured sphere
	spheres.emplace_back(std::make_shared<Sphere>(
		glm::vec3(1.0f, -0.95f, -2.75f),
		glm::vec3(0.0f),
		0.45f,
		Material{
			.texture = {
				.type = TextureType::Image
			},
			.type = MaterialType::Lambertian,
		}));

	// Noise sphere
	spheres.emplace_back(std::make_shared<Sphere>(
		glm::vec3(-0.5f, -0.95f, -4.45f),
		glm::vec3(0.0f),
		0.45f,
		Material{
			.texture = {
				.type = TextureType::Noise,
				.noise = glm::vec4(7.0f, 0.5f, 2.0f, 0.0f)
			},
			.type = MaterialType::Lambertian,
		}));

	std::vector<std::shared_ptr<Hittable> > volumes;

	auto sphere = std::make_shared<Sphere>(
		glm::vec3(-0.75f, -0.95f, -2.75f),
		glm::vec3(0.0f),
		0.45f,
		Material{
			.type = MaterialType::Dielectric,
			.refractionIndex = 1.5f
		});

	volumes.emplace_back(std::make_shared<Volume>(
		sphere,
		0.2f,
		Material{
			.texture = {
				.type = TextureType::SolidColor,
				.color = glm::vec3(0.2f, 0.4f, 0.9f)
			},
			.type = MaterialType::Isotropic
		}));

	spheres.push_back(sphere);

	std::vector<std::shared_ptr<Hittable> > box = Quad::box(
		glm::vec3(0.15f, -1.5f, -4.35f), // min corner
		glm::vec3(1.15f, 0.0f, -3.25f), // max corner
		0.0f,
		Material{
			.texture = {
				.type = TextureType::SolidColor,
				.color = glm::vec3(0.73f)
			},
			.type = MaterialType::Lambertian
		});


	std::vector<std::shared_ptr<Hittable> > hittables = spheres;
	hittables.insert(hittables.end(), cornellBox.begin(), cornellBox.end());
	hittables.insert(hittables.end(), volumes.begin(), volumes.end());

	return {
		.bvh = std::make_shared<BVHNode>(hittables, 0, hittables.size()),
		.sphereCount = spheres.size(),
		.quadCount = cornellBox.size(),
		.volumeCount = volumes.size(),
		.bvhNodeCount = BVHNode::count,
	};
}
