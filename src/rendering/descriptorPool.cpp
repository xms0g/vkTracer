#include "descriptorPool.h"

DescriptorPool::DescriptorPool(const vk::raii::Device& device) : mDevice(&device) {
}

DescriptorPool& DescriptorPool::addMaxSets(const uint32_t count) {
	mMaxSets = count;
	return *this;
}

DescriptorPool& DescriptorPool::addPoolFlags(const vk::DescriptorPoolCreateFlags flags) {
	mPoolFlags = flags;
	return *this;
}

DescriptorPool& DescriptorPool::addPoolSize(vk::DescriptorType type, uint32_t count) {
	mPoolSizes.emplace_back(type, count);
	return *this;
}

void DescriptorPool::build() {
	const vk::DescriptorPoolCreateInfo poolInfo{
		.flags = mPoolFlags,
		.maxSets = mMaxSets,
		.poolSizeCount = static_cast<uint32_t>(mPoolSizes.size()),
		.pPoolSizes = mPoolSizes.data()
	};

	mDescriptorPool = vk::raii::DescriptorPool(*mDevice, poolInfo);
}
