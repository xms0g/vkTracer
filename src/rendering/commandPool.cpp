#include "commandPool.hpp"

CommandPool::CommandPool(const vk::raii::Device& device, const uint32_t queueIndex, const vk::CommandPoolCreateFlags flags) {
	const vk::CommandPoolCreateInfo poolInfo{
		.flags = flags,
		.queueFamilyIndex = queueIndex
	};

	mCommandPool = vk::raii::CommandPool(device, poolInfo);
}
