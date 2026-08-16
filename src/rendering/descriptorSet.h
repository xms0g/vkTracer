#pragma once
#include <vector>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class DescriptorPool;

class DescriptorSetAllocator {
public:
	DescriptorSetAllocator(const vk::raii::Device& device, const DescriptorPool& pool);

	vk::raii::DescriptorSets allocate(uint32_t count, const vk::DescriptorSetLayout& layout) const;

private:
	const DescriptorPool& mPool;
	const vk::raii::Device& mDevice;
};

class DescriptorSetWriter {
public:
	explicit DescriptorSetWriter(const vk::raii::Device& device);

	void reserve(uint32_t count);

	DescriptorSetWriter& writeBuffer(
	   vk::DescriptorSet set,
	   uint32_t binding,
	   vk::DescriptorType type,
	   vk::Buffer buffer,
	   vk::DeviceSize offset,
	   vk::DeviceSize range);

	void update();

	void flush();

private:
	std::vector<vk::WriteDescriptorSet> mWrites;
	std::vector<vk::DescriptorBufferInfo> mBufferInfos;
	const vk::raii::Device& mDevice;
};
