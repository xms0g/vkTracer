#pragma once
#include <cstdint>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class DeviceMemory {
public:
	DeviceMemory() = default;

	explicit DeviceMemory(const vk::raii::Device& device,
						  const vk::raii::PhysicalDevice& phyDev,
						  vk::DeviceSize size,
						  uint32_t typeFilter,
						  vk::MemoryPropertyFlags properties);

	vk::raii::DeviceMemory& operator*() noexcept { return mMemory; }
	const vk::raii::DeviceMemory& operator*() const noexcept { return mMemory; }

private:
	static uint32_t findMemoryType(uint32_t typeFilter,
								   vk::MemoryPropertyFlags properties,
								   const vk::raii::PhysicalDevice& phyDev);

	vk::raii::DeviceMemory mMemory{nullptr};
};