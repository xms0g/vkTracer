#include "descriptorSetLayout.h"

DescriptorSetLayout::DescriptorSetLayout(const vk::raii::Device& device) : mDevice(&device) {
}

DescriptorSetLayout& DescriptorSetLayout::addBinding(
	uint32_t binding,
	vk::DescriptorType type,
	uint32_t count,
	vk::ShaderStageFlags stageFlags) {
	mBindings.emplace_back(binding, type, count, stageFlags);
	return *this;
}

void DescriptorSetLayout::build() {
	const vk::DescriptorSetLayoutCreateInfo layoutInfo{
		.bindingCount = static_cast<uint32_t>(mBindings.size()),
		.pBindings = mBindings.data()
	};

	mDescriptorSetLayout = vk::raii::DescriptorSetLayout(*mDevice, layoutInfo);
}
