#pragma once
#include <vector>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class DescriptorPool {
public:
	DescriptorPool() = default;

	explicit DescriptorPool(const vk::raii::Device& device);

	DescriptorPool& addMaxSets(uint32_t count);

	DescriptorPool& addPoolFlags(vk::DescriptorPoolCreateFlags flags);

	DescriptorPool& addPoolSize(vk::DescriptorType type, uint32_t count);

	void build();

	vk::raii::DescriptorPool& operator*() noexcept { return mDescriptorPool; }
	const vk::raii::DescriptorPool& operator*() const noexcept { return mDescriptorPool; }

private:
	uint32_t mMaxSets{0};
	const vk::raii::Device* mDevice;
	std::vector<vk::DescriptorPoolSize> mPoolSizes;
	vk::DescriptorPoolCreateFlags mPoolFlags;
	vk::raii::DescriptorPool mDescriptorPool{nullptr};
};
