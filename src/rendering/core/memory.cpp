#include "memory.hpp"

DeviceMemory::DeviceMemory(const vk::raii::Device& device,
                           const vk::raii::PhysicalDevice& phyDev,
                           const vk::DeviceSize size,
                           const uint32_t typeFilter,
                           const vk::MemoryPropertyFlags properties,
                           const vk::MemoryAllocateFlagsInfo& allocFlags) {

	const vk::MemoryAllocateInfo allocInfo{
		.pNext = &allocFlags,
		.allocationSize = size,
		.memoryTypeIndex = findMemoryType(typeFilter, properties, phyDev)
	};

	mMemory = vk::raii::DeviceMemory(device, allocInfo);
}

DeviceMemory::DeviceMemory(DeviceMemory&& other) noexcept
	: mMemory(std::move(other.mMemory)) {
}

DeviceMemory& DeviceMemory::operator=(DeviceMemory&& other) noexcept {
	if (this != &other) {
		mMemory = std::move(other.mMemory);
	}

	return *this;
}

uint32_t DeviceMemory::findMemoryType(const uint32_t typeFilter,
                                      const vk::MemoryPropertyFlags properties,
                                      const vk::raii::PhysicalDevice& phyDev) {
	const vk::PhysicalDeviceMemoryProperties memProperties = phyDev.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}
