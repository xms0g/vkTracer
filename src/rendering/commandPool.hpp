#pragma once
#include <cstdint>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class CommandPool {
public:
	CommandPool() = default;

	explicit CommandPool(const vk::raii::Device& device, uint32_t queueIndex = 0, vk::CommandPoolCreateFlags flags = {});

	vk::raii::CommandPool& operator*() noexcept { return mCommandPool; }
	const vk::raii::CommandPool& operator*() const noexcept { return mCommandPool; }

private:
	vk::raii::CommandPool mCommandPool{nullptr};
};
