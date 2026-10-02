#pragma once

template<QueueType T>
void Device::submit() {
	const CommandBuffer* commandBuffer;
	uint64_t waitValue;
	uint64_t signalValue;
	vk::PipelineStageFlagBits2 stage;

	if constexpr (T == QueueType::Compute) {
		recordComputeCommandBuffer();

		commandBuffer = &mComputeCommandBuffers[mFrameIndex];
		waitValue = mComputeWaitValue;
		signalValue = mComputeSignalValue;
		stage = vk::PipelineStageFlagBits2::eComputeShader;

		mCurrentTiles[mFrameIndex] += TILES_PER_FRAME;

		if (mCurrentTiles[mFrameIndex] >= TILE_COUNT)
			mCurrentTiles[mFrameIndex] = 0;
	} else {
		recordGraphicsCommandBuffer(mImageIndex);
		
		commandBuffer = &mGraphicsCommandBuffers[mFrameIndex];
		waitValue = mGraphicsWaitValue;
		signalValue = mGraphicsSignalValue;
		stage = vk::PipelineStageFlagBits2::eFragmentShader;
	}

	vk::SemaphoreSubmitInfo wait{
		.semaphore = *mSemaphore,
		.value = waitValue,
		.stageMask = stage
	};

	vk::CommandBufferSubmitInfo command{
		.commandBuffer = ***commandBuffer
	};

	vk::SemaphoreSubmitInfo signal{
		.semaphore = *mSemaphore,
		.value = signalValue,
		.stageMask = stage
	};

	const vk::SubmitInfo2 submitInfo{
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &wait,

		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &command,

		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signal
	};

	mQueue.submit2(submitInfo);
}

