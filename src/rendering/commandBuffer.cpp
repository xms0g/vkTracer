#include "commandBuffer.h"
#include "commandPool.h"

CommandBuffer::CommandBuffer(const vk::raii::Device& device, const CommandPool& commandPool, const vk::CommandBufferLevel level) {
	const vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = **commandPool,
		.level = level,
		.commandBufferCount = 1
	};

	mCommandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
}
