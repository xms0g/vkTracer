#include "buffer.h"

Buffer::Buffer(
	const vk::DeviceSize size,
	const vk::raii::Device& device,
	const vk::raii::PhysicalDevice& phyDev,
	const vk::BufferUsageFlags usage,
	const vk::MemoryPropertyFlags properties)
	: mSize(size) {
	const vk::BufferCreateInfo bufferInfo{
		.size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive
	};

	mBuffer = vk::raii::Buffer(device, bufferInfo);

	const vk::MemoryRequirements memRequirements = mBuffer.getMemoryRequirements();
	const vk::MemoryAllocateInfo allocInfo{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, phyDev)
	};

	mBufferMemory = vk::raii::DeviceMemory(device, allocInfo);
	mBuffer.bindMemory(mBufferMemory, 0);
}

vk::DeviceSize Buffer::size() const {
	return mSize;
}

void* Buffer::mappedMemory() const {
	return mMappedMemory;
}

void* Buffer::map(const size_t size) {
	mMappedMemory = mBufferMemory.mapMemory(0, size);
	return mMappedMemory;
}

void Buffer::unmap() const {
	mBufferMemory.unmapMemory();
}

uint32_t Buffer::findMemoryType(
	const uint32_t typeFilter,
	const vk::MemoryPropertyFlags properties,
	const vk::raii::PhysicalDevice& phyDev) {
	const vk::PhysicalDeviceMemoryProperties memProperties = phyDev.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}
