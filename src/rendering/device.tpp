#pragma once

template<QueueType T>
void Device::submit(const float deltaTime) {
	const CommandBuffer* commandBuffer;
	uint64_t waitValue;
	uint64_t signalValue;
	vk::PipelineStageFlags waitStage;

	if constexpr (T == QueueType::Compute) {
		recordComputeCommandBuffer(deltaTime);
		commandBuffer = &mComputeCommandBuffers[mFrameIndex];
		waitValue = mComputeWaitValue;
		signalValue = mComputeSignalValue;
		waitStage = vk::PipelineStageFlagBits::eComputeShader;
	} else {
		recordGraphicsCommandBuffer(mImageIndex);
		commandBuffer = &mGraphicsCommandBuffers[mFrameIndex];
		waitValue = mGraphicsWaitValue;
		signalValue = mGraphicsSignalValue;
		waitStage = vk::PipelineStageFlagBits::eVertexInput;
	}

	vk::TimelineSemaphoreSubmitInfo timelineInfo{
		.waitSemaphoreValueCount = 1,
		.pWaitSemaphoreValues = &waitValue,
		.signalSemaphoreValueCount = 1,
		.pSignalSemaphoreValues = &signalValue
	};

	const vk::SubmitInfo submitInfo{
		.pNext = &timelineInfo,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*mSemaphore,
		.pWaitDstStageMask = &waitStage,
		.commandBufferCount = 1,
		.pCommandBuffers = &***commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*mSemaphore
	};

	mQueue.submit(submitInfo, nullptr);
}

