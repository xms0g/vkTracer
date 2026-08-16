#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Buffer {
public:
	Buffer(
		vk::DeviceSize size,
		const vk::raii::Device& device,
		const vk::raii::PhysicalDevice& phyDev,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties);

	[[nodiscard]]
	vk::DeviceSize size() const;

	[[nodiscard]]
	void* mappedMemory() const;

	[[nodiscard]]
	void* map(size_t size);

	void unmap() const;

	vk::raii::Buffer& operator*() noexcept { return mBuffer; }
	const vk::raii::Buffer& operator*() const noexcept { return mBuffer; }

private:
	static uint32_t findMemoryType(
		uint32_t typeFilter,
		vk::MemoryPropertyFlags properties,
		const vk::raii::PhysicalDevice& phyDev);

	vk::DeviceSize mSize;
	vk::raii::Buffer mBuffer{nullptr};
	vk::raii::DeviceMemory mBufferMemory{nullptr};
	void* mMappedMemory{nullptr};
};
