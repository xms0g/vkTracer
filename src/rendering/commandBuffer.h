#pragma once
#include <cstdint>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class CommandPool;

class CommandBuffer {
public:
	CommandBuffer(const vk::raii::Device& device, const CommandPool& commandPool, vk::CommandBufferLevel level);

	vk::raii::CommandBuffer& operator*() noexcept { return mCommandBuffer; }
	const vk::raii::CommandBuffer& operator*() const noexcept { return mCommandBuffer; }

private:
	vk::raii::CommandBuffer mCommandBuffer{nullptr};

};
