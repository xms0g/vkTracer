#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include "commandBuffer.h"
#include "commandPool.h"
#include "descriptorPool.h"
#include "descriptorSetLayout.h"
#include "pipelineBuilder.h"
#include "swapchain.h"

enum class QueueType { Graphics, Compute };

class Window;
class Buffer;

class Device {
public:
	explicit Device(Window& window);

	~Device();

	void init();

	void prepareFrame();

	template<QueueType T>
	void submit(float deltaTime);

	void presentFrame();

	void waitIdle() const;

private:
	// Getters
	void getPhysicalDevice();

	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void createLogicalDevice();

	void createSwapchain();

	void createDescriptorSetLayout();

	void createPipelines();

	void createCommandPool();

	void createDescriptorPool();

	void createShaderStorageBuffers();

	void createComputeDescriptorSets();

	void createCommandBuffers();

	void recordGraphicsCommandBuffer(uint32_t imageIndex);

	void recordComputeCommandBuffer(float deltaTime);

	void createSyncObjects();

	// Support Functions
	static std::vector<const char*> getRequiredInstanceExtensions();

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		vk::DebugUtilsMessageTypeFlagsEXT type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	static bool checkDeviceSuitable(const vk::raii::PhysicalDevice& phyDevice);

	void copyBuffer(const Buffer& dstBuffer, const Buffer& srcBuffer, vk::DeviceSize size) const;

	[[nodiscard]]
	vk::raii::CommandBuffer beginSingleTimeCommands() const;

	void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const;

	struct ComputePushConstants {
		float deltaTime{1.0f};
	};

	Window& mWindow;
	uint32_t mImageIndex{0};
	uint32_t mFrameIndex{0};
	uint64_t mTimelineValue{0};
	uint64_t mComputeWaitValue{0};
	uint64_t mComputeSignalValue{0};
	uint64_t mGraphicsWaitValue{0};
	uint64_t mGraphicsSignalValue{0};
	vk::raii::Context mContext;
	vk::raii::Instance mInstance{nullptr};
	vk::raii::PhysicalDevice mPhysicalDevice{nullptr};
	vk::raii::Device mDevice{nullptr};
	vk::raii::Queue mQueue{nullptr};
	uint32_t mQueueIndex{static_cast<uint32_t>(~0)};
	vk::raii::SurfaceKHR mSurface{nullptr};
	Swapchain mSwapchain{};
	DescriptorSetLayout mComputeDescriptorSetLayout{};
	DescriptorPool mDescriptorPool{};
	std::vector<vk::raii::DescriptorSet> mComputeDescriptorSets;
	GraphicsPipeline mGraphicsPipeline{};
	ComputePipeline mComputePipeline{};
	std::vector<Buffer> mShaderStorageBuffers;
	CommandPool mCommandPool;
	std::vector<CommandBuffer> mGraphicsCommandBuffers;
	std::vector<CommandBuffer> mComputeCommandBuffers;
	vk::raii::Semaphore mSemaphore{nullptr};
	std::vector<vk::raii::Fence> mFences;
	vk::raii::DebugUtilsMessengerEXT mDebugMessenger{nullptr};
};

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
