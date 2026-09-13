#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include "commandBuffer.hpp"
#include "commandPool.hpp"
#include "descriptorPool.hpp"
#include "descriptorSetLayout.hpp"
#include "pipelineBuilder.hpp"
#include "swapchain.hpp"

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

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
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

#include "device.tpp"
