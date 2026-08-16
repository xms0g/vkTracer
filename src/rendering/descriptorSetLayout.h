#pragma once
#include <vector>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class DescriptorSetLayout {
public:
	DescriptorSetLayout() = default;

	explicit DescriptorSetLayout(const vk::raii::Device& device);

	DescriptorSetLayout& addBinding(
		uint32_t binding,
		vk::DescriptorType type,
		uint32_t count = 1,
		vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll);

	void build();

	vk::raii::DescriptorSetLayout& operator*() noexcept { return mDescriptorSetLayout; }
	const vk::raii::DescriptorSetLayout& operator*() const noexcept { return mDescriptorSetLayout; }

private:
	const vk::raii::Device* mDevice;
	std::vector<vk::DescriptorSetLayoutBinding> mBindings;
	vk::raii::DescriptorSetLayout mDescriptorSetLayout{nullptr};
};
