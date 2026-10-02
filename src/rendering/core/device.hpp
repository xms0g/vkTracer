#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>
#include "commandBuffer.hpp"
#include "commandPool.hpp"
#include "descriptorPool.hpp"
#include "descriptorSetLayout.hpp"
#include "image.hpp"
#include "pipelineBuilder.hpp"
#include "swapchain.hpp"
#include "../../config/config.hpp"

class Camera;
class Image;
class DeviceMemory;

enum class QueueType { Graphics, Compute };

class Window;
class Buffer;

class Device {
public:
	explicit Device(Window& window, Camera& camera);

	~Device();

	void init();

	void prepareFrame();

	template<QueueType T>
	void submit();

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

	void createShaderStorageImage();

	void createTextureImage(std::string_view path);

	void createSamplers();

	void createDescriptorSets();

	void createCommandBuffers();

	void recordGraphicsCommandBuffer(uint32_t imageIndex);

	void recordComputeCommandBuffer();

	void createSyncObjects();

	void copyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, vk::DeviceSize size) const;

	void copyBufferToImage(const Buffer& srcBuffer,
	                       const Image& dstImage,
	                       uint32_t width,
	                       uint32_t height) const;

	[[nodiscard]]
	CommandBuffer beginSingleTimeCommands() const;

	void endSingleTimeCommands(const CommandBuffer& commandBuffer) const;

	// Support Functions
	[[nodiscard]]
	std::vector<const char*> getRequiredInstanceExtensions() const;

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	                                                      vk::DebugUtilsMessageTypeFlagsEXT type,
	                                                      const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	                                                      void* pUserData);

	static bool checkDeviceSuitable(const vk::raii::PhysicalDevice& phyDevice);

	struct ComputePushConstants {
		uint64_t bufferAddress;
		uint32_t currentTile;
		alignas(16) glm::vec4 resolution;
		alignas(16) glm::vec4 camCenter;
		alignas(16) glm::vec4 camFront;
		alignas(16) glm::vec4 camRight;
		alignas(16) glm::vec4 camUp;
	};

	Window& mWindow;
	Camera& mCamera;
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
	DescriptorSetLayout mGraphicsDescriptorSetLayout{};
	DescriptorPool mDescriptorPool{};
	std::vector<vk::raii::DescriptorSet> mComputeDescriptorSets;
	std::vector<vk::raii::DescriptorSet> mGraphicsDescriptorSets;
	GraphicsPipeline mGraphicsPipeline{};
	ComputePipeline mComputePipeline{};
	std::vector<Image> mShaderStorageImages;
	std::vector<Image> mTextureImages;
	std::vector<Buffer> mShaderStorageBuffers;
	std::vector<uint64_t> mShaderStorageBufferAddresses;
	std::vector<vk::raii::Sampler> mSamplers;
	CommandPool mCommandPool;
	std::vector<CommandBuffer> mGraphicsCommandBuffers;
	std::vector<CommandBuffer> mComputeCommandBuffers;
	vk::raii::Semaphore mSemaphore{nullptr};
	std::vector<vk::raii::Fence> mFences;
	std::array<uint32_t, MAX_FRAMES_IN_FLIGHT> mCurrentTiles{};
	vk::raii::DebugUtilsMessengerEXT mDebugMessenger{nullptr};
};

#include "device.tpp"
