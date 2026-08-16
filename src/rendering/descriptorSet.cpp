#include "descriptorSet.h"
#include "descriptorPool.h"

DescriptorSetAllocator::DescriptorSetAllocator(const vk::raii::Device& device, const DescriptorPool& pool)
	: mPool(pool), mDevice(device) {
}

vk::raii::DescriptorSets DescriptorSetAllocator::allocate(const uint32_t count, const vk::DescriptorSetLayout& layout) const {
	std::vector<vk::DescriptorSetLayout> layouts(count, layout);

	const vk::DescriptorSetAllocateInfo allocInfo{
		.descriptorPool = **mPool,
		.descriptorSetCount = count,
		.pSetLayouts = layouts.data()
	};

	return mDevice.allocateDescriptorSets(allocInfo);
}

DescriptorSetWriter::DescriptorSetWriter(const vk::raii::Device& device) : mDevice(device) {
}

void DescriptorSetWriter::reserve(const uint32_t count) {
	mBufferInfos.reserve(count);
	mWrites.reserve(count);
}

DescriptorSetWriter& DescriptorSetWriter::writeBuffer(
	const vk::DescriptorSet set,
	const uint32_t binding,
	const vk::DescriptorType type,
	const vk::Buffer buffer,
	const vk::DeviceSize offset,
	const vk::DeviceSize range) {
	mBufferInfos.emplace_back(vk::DescriptorBufferInfo{
		 .buffer = buffer,
		 .offset = offset,
		 .range = range
	 });

	mWrites.emplace_back(vk::WriteDescriptorSet{
		.dstSet = set,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = type,
		.pBufferInfo = &mBufferInfos.back()
	});

	return *this;
}

void DescriptorSetWriter::update() {
	mDevice.updateDescriptorSets(mWrites, nullptr);
	flush();
}

void DescriptorSetWriter::flush() {
	mBufferInfos.clear();
	mWrites.clear();
}
