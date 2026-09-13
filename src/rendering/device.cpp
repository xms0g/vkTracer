#include "device.hpp"
#include <set>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <random>
#include <unordered_set>
#include <GLFW/glfw3.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "particle.hpp"
#include "buffer.hpp"
#include "swapchain.hpp"
#include "commandPool.hpp"
#include "descriptorPool.hpp"
#include "descriptorSet.hpp"
#include "deviceExtension.hpp"
#include "image.hpp"
#include "memory.hpp"
#include "pipelineBuilder.hpp"
#include "validation.hpp"
#include "../core/window.hpp"
#include "../config/config.hpp"

Device::Device(Window& window) : mWindow(window) {
}

Device::~Device() = default;

void Device::init() {
	try {
		createInstance();
		setupDebugMessenger();
		createSurface();
		getPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
		createDescriptorSetLayout();
		createPipelines();
		createCommandPool();
		createShaderStorageImage();
		createSampler();
		createDescriptorPool();
		createComputeDescriptorSets();
		createGraphicsDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	} catch (const std::runtime_error& e) {
		throw std::runtime_error(e.what());
	}
}

void Device::prepareFrame() {
	mImageIndex = mSwapchain.acquireNextImage(mFences[mFrameIndex]);

	auto fenceResult = mDevice.waitForFences(*mFences[mFrameIndex], vk::True, UINT64_MAX);
	if (fenceResult != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to wait for fence!");
	}

	mDevice.resetFences(*mFences[mFrameIndex]);

	mComputeWaitValue = mTimelineValue;
	mComputeSignalValue = ++mTimelineValue;
	mGraphicsWaitValue = mComputeSignalValue;
	mGraphicsSignalValue = ++mTimelineValue;
}

void Device::presentFrame() {
	// Present the image (wait for graphics to finish)
	const vk::SemaphoreWaitInfo waitInfo{
		.semaphoreCount = 1,
		.pSemaphores = &*mSemaphore,
		.pValues = &mGraphicsSignalValue
	};

	// Wait for graphics to complete before presenting
	auto result = mDevice.waitSemaphores(waitInfo, UINT64_MAX);
	if (result != vk::Result::eSuccess) {
		throw std::runtime_error("failed to wait for semaphore!");
	}

	const vk::PresentInfoKHR presentInfo{
		.waitSemaphoreCount = 0, // No binary semaphores needed
		.pWaitSemaphores = nullptr,
		.swapchainCount = 1,
		.pSwapchains = &**mSwapchain,
		.pImageIndices = &mImageIndex
	};

	result = mQueue.presentKHR(presentInfo);
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || mWindow.windowResized()) {
		mWindow.windowResized(false);
		mSwapchain.recreate(mSurface, mDevice, mPhysicalDevice, *mWindow);
	} else {
		// There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
		assert(result == vk::Result::eSuccess);
	}

	mFrameIndex = (mFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Device::waitIdle() const {
	mDevice.waitIdle();
}

void Device::getPhysicalDevice() {
	const auto physicalDevices = mInstance.enumeratePhysicalDevices();

	if (physicalDevices.empty()) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	const auto deviceIt = std::ranges::find_if(
		physicalDevices, [&](const auto& phyDevice) { return checkDeviceSuitable(phyDevice); });

	if (deviceIt != physicalDevices.end()) {
		mPhysicalDevice = *deviceIt;
	}
}

void Device::createInstance() {
	constexpr vk::ApplicationInfo appInfo{
		.pApplicationName = "Vk Tracer",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	};

	if (enableValidationLayers) {
		std::unordered_set<std::string> supportedValidationLayers;
		for (const auto& layer: mContext.enumerateInstanceLayerProperties()) {
			supportedValidationLayers.insert(layer.layerName);
		}

		bool supportedRequiredValidationLayers = std::ranges::all_of(validationLayers, [&](const auto& layer) {
			return supportedValidationLayers.contains(layer);
		});

		if (!supportedRequiredValidationLayers) {
			throw std::runtime_error("Required Validation Layers not supported");
		}
	}

	const auto glfwExtensions = getRequiredInstanceExtensions();

	std::unordered_set<std::string> supportedExtensions;
	for (const auto& [extensionName, specVersion]: mContext.enumerateInstanceExtensionProperties()) {
		supportedExtensions.insert(extensionName);
	}

	std::vector<const char*> requiredExtensions;
	for (const auto& extension: glfwExtensions) {
		if (!supportedExtensions.contains(extension)) {
			throw std::runtime_error("Required GLFW extension not supported: " + std::string(extension));
		}
		requiredExtensions.emplace_back(extension);
	}

	vk::InstanceCreateFlagBits flags{0};
#ifdef __APPLE__
	requiredExtensions.emplace_back(vk::KHRPortabilityEnumerationExtensionName);
	flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

	const vk::InstanceCreateInfo createInfo{
		.flags = flags,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data()
	};

	mInstance = vk::raii::Instance(mContext, createInfo);
}

void Device::setupDebugMessenger() {
	if constexpr (!enableValidationLayers) {
		return;
	}

	constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
	};

	constexpr vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
	};

	constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
		.messageSeverity = severityFlags,
		.messageType = messageTypeFlags,
		.pfnUserCallback = &debugCallback
	};

	mDebugMessenger = mInstance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void Device::createSurface() {
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(*mInstance, &*mWindow, nullptr, &surface) != 0) {
		throw std::runtime_error("Failed to create window surface!");
	}

	mSurface = vk::raii::SurfaceKHR(mInstance, surface);
}

void Device::createLogicalDevice() {
	const std::vector<vk::QueueFamilyProperties> queueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics &&
		    queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute &&
		    mPhysicalDevice.getSurfaceSupportKHR(i, *mSurface)) {
			mQueueIndex = i;
			break;
		}
	}

	if (mQueueIndex == ~0) {
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	// Create a chain of feature structures
	const vk::StructureChain<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
		vk::PhysicalDeviceTimelineSemaphoreFeaturesKHR> featureChain = {
		{.features = {.samplerAnisotropy = true}},
		{.shaderDrawParameters = true},
		{.synchronization2 = true, .dynamicRendering = true},
		{.extendedDynamicState = true},
		{.timelineSemaphore = true}
	};

	float queuePriority = 0.5f;
	const vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
		.queueFamilyIndex = mQueueIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	const vk::DeviceCreateInfo deviceCreateInfo{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data()
	};

	mDevice = vk::raii::Device(mPhysicalDevice, deviceCreateInfo);
	mQueue = vk::raii::Queue(mDevice, mQueueIndex, 0);
}

void Device::createSwapchain() {
	mSwapchain = Swapchain(mSurface, mDevice, mPhysicalDevice, *mWindow);
}

void Device::createDescriptorSetLayout() {
	mComputeDescriptorSetLayout = DescriptorSetLayout(mDevice);
	mComputeDescriptorSetLayout
			.addBinding(
				0,
				vk::DescriptorType::eStorageImage,
				1,
				vk::ShaderStageFlagBits::eCompute)
			.build();

	mGraphicsDescriptorSetLayout = DescriptorSetLayout(mDevice);
	mGraphicsDescriptorSetLayout
			.addBinding(
				0,
				vk::DescriptorType::eCombinedImageSampler,
				1,
				vk::ShaderStageFlagBits::eFragment)
			.build();
}

void Device::createPipelines() {
	PipelineBuilder builder{mDevice};
	Shader shader{mDevice, std::string(SHADER_BINARY_DIR) + SHADER_NAME};

	mGraphicsPipeline = GraphicsPipeline(
		builder,
		shader,
		mSwapchain.surfaceFormat(),
		mGraphicsDescriptorSetLayout,
		1);

	mComputePipeline = ComputePipeline(
		builder,
		shader,
		mComputeDescriptorSetLayout,
		1,
		sizeof(ComputePushConstants));
}

void Device::createCommandPool() {
	mCommandPool = CommandPool(
		mDevice,
		mQueueIndex,
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
}

void Device::createDescriptorPool() {
	mDescriptorPool = DescriptorPool(mDevice);
	mDescriptorPool
			.addMaxSets(MAX_FRAMES_IN_FLIGHT * 2)
			.addPoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
			.addPoolSize(vk::DescriptorType::eStorageImage, MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT)
			.build();
}

void Device::createShaderStorageImage() {
	mShaderStorageImages.clear();
	mShaderStorageImageMemory.clear();
	mShaderStorageImageViews.clear();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vk::ImageCreateInfo imageInfo{};
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = vk::Format::eR8G8B8A8Unorm;
		imageInfo.extent = vk::Extent3D{
			.width = WIDTH,
			.height = HEIGHT,
			.depth = 1
		};
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		mShaderStorageImages.emplace_back(mDevice, imageInfo);
		auto& image = mShaderStorageImages.back();

		const vk::MemoryRequirements memRequirements = image.getMemoryRequirements();

		mShaderStorageImageMemory.emplace_back(mDevice,
			mPhysicalDevice,
			memRequirements.size,
			memRequirements.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

		auto& memory = mShaderStorageImageMemory.back();
		image.bindMemory(*memory, 0);

		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = vk::Format::eR8G8B8A8Unorm;

		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;

		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		mShaderStorageImageViews.emplace_back(mDevice, viewInfo);
	}
}

void Device::createSampler() {
	constexpr vk::SamplerCreateInfo samplerInfo{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eNearest,
		.addressModeU = vk::SamplerAddressMode::eClampToEdge,
		.addressModeV = vk::SamplerAddressMode::eClampToEdge,
		.addressModeW = vk::SamplerAddressMode::eClampToEdge,
		.mipLodBias = 0.0f,
		.anisotropyEnable = vk::False,
		.maxAnisotropy = 1.0f,
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.0f,
		.maxLod = 0.0f
	};

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		mSamplers.emplace_back(mDevice, samplerInfo);
	}
}

void Device::createComputeDescriptorSets() {
	const DescriptorSetAllocator allocator(mDevice, mDescriptorPool);
	mComputeDescriptorSets = allocator.allocate(MAX_FRAMES_IN_FLIGHT, **mComputeDescriptorSetLayout);

	DescriptorSetWriter writer(mDevice);
	writer.reserve(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		writer.writeImage(
			*mComputeDescriptorSets[i],
			0,
			vk::DescriptorType::eStorageImage,
			mShaderStorageImageViews[i],
			vk::ImageLayout::eGeneral);

		writer.update();
	}
}

void Device::createGraphicsDescriptorSets() {
	const DescriptorSetAllocator allocator(mDevice, mDescriptorPool);
	mGraphicsDescriptorSets = allocator.allocate(MAX_FRAMES_IN_FLIGHT, **mGraphicsDescriptorSetLayout);

	DescriptorSetWriter writer(mDevice);
	writer.reserve(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		writer.writeImage(
			*mGraphicsDescriptorSets[i],
			0,
			vk::DescriptorType::eCombinedImageSampler,
			mShaderStorageImageViews[i],
			vk::ImageLayout::eShaderReadOnlyOptimal,
			mSamplers[i]);

		writer.update();
	}
}

void Device::createCommandBuffers() {
	mGraphicsCommandBuffers.clear();
	mComputeCommandBuffers.clear();

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		mGraphicsCommandBuffers.emplace_back(mDevice, mCommandPool, vk::CommandBufferLevel::ePrimary);
		mComputeCommandBuffers.emplace_back(mDevice, mCommandPool, vk::CommandBufferLevel::ePrimary);
	}
}

void Device::recordGraphicsCommandBuffer(const uint32_t imageIndex) {
	const auto& commandBuffer = mGraphicsCommandBuffers[mFrameIndex];
	(*commandBuffer).reset();
	(*commandBuffer).begin({});

	const auto& image = mSwapchain.image(imageIndex);
	// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
	image::transitionImageLayout(
		image,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{}, // srcAccessMask (no need to wait for previous operations)
		vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, // dstStage
		commandBuffer
	);

	constexpr vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

	vk::RenderingAttachmentInfo attachmentInfo = {
		.imageView = mSwapchain.imageView(imageIndex),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
	};

	const vk::RenderingInfo renderingInfo = {
		.renderArea = {.offset = {0, 0}, .extent = mSwapchain.extent()},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentInfo
	};

	(*commandBuffer).beginRendering(renderingInfo);
	(*commandBuffer).bindPipeline(vk::PipelineBindPoint::eGraphics, **mGraphicsPipeline);
	(*commandBuffer).bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		mGraphicsPipeline.layout(),
		0,
		{mGraphicsDescriptorSets[mFrameIndex]},
		{});
	(*commandBuffer).setViewport(
		0,
		vk::Viewport(
			0.0f,
			0.0f,
			static_cast<float>(mSwapchain.extent().width),
			static_cast<float>(mSwapchain.extent().height),
			0.0f,
			1.0f));
	(*commandBuffer).setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), mSwapchain.extent()));
	(*commandBuffer).draw(3, 1, 0, 0);
	(*commandBuffer).endRendering();
	// After rendering, transition the swapchain image to PRESENT_SRC
	image::transitionImageLayout(
		image,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite, // srcAccessMask
		{}, // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe, // dstStage
		commandBuffer
	);
	(*commandBuffer).end();
}

void Device::recordComputeCommandBuffer() {
	const auto& commandBuffer = mComputeCommandBuffers[mFrameIndex];
	(*commandBuffer).reset();
	(*commandBuffer).begin({});

	const vk::ImageMemoryBarrier2 barrier{
		.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
		.srcAccessMask = {},
		.dstStageMask = vk::PipelineStageFlagBits2::eComputeShader,
		.dstAccessMask = vk::AccessFlagBits2::eShaderWrite,
		.oldLayout = vk::ImageLayout::eUndefined,
		.newLayout = vk::ImageLayout::eGeneral,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = mShaderStorageImages[mFrameIndex],
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	const vk::DependencyInfo dependencyInfo{
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};

	(*commandBuffer).pipelineBarrier2(dependencyInfo);

	(*commandBuffer).bindPipeline(vk::PipelineBindPoint::eCompute, **mComputePipeline);
	(*commandBuffer).bindDescriptorSets(
		vk::PipelineBindPoint::eCompute,
		mComputePipeline.layout(),
		0,
		{mComputeDescriptorSets[mFrameIndex]}, {});

	constexpr ComputePushConstants pc{
		.width = WIDTH,
		.height = HEIGHT,
	};

	(*commandBuffer).pushConstants(
		mComputePipeline.layout(),
		vk::ShaderStageFlagBits::eCompute,
		0,
		vk::ArrayProxy<const ComputePushConstants>(pc));
	(*commandBuffer).dispatch(256 / 8, 256 / 8, 1);

	const vk::ImageMemoryBarrier2 barrier1{
		.srcStageMask = vk::PipelineStageFlagBits2::eComputeShader,
		.srcAccessMask = vk::AccessFlagBits2::eShaderWrite,
		.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
		.dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead,
		.oldLayout = vk::ImageLayout::eGeneral,
		.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = mShaderStorageImages[mFrameIndex],
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	const vk::DependencyInfo dependencyInfo1{
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier1
	};

	(*commandBuffer).pipelineBarrier2(dependencyInfo1);

	(*commandBuffer).end();
}

void Device::createSyncObjects() {
	mFences.clear();

	constexpr vk::SemaphoreTypeCreateInfo semaphoreType{
		.semaphoreType = vk::SemaphoreType::eTimeline,
		.initialValue = 0
	};
	mSemaphore = vk::raii::Semaphore(mDevice, {.pNext = &semaphoreType});
	mTimelineValue = 0;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vk::FenceCreateInfo fenceInfo{};
		mFences.emplace_back(mDevice, fenceInfo);
	}
}

std::vector<const char*> Device::getRequiredInstanceExtensions() {
	uint32_t glfwExtensionCount = 0;
	const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers) {
		extensions.push_back(vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}

vk::Bool32 Device::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                 const vk::DebugUtilsMessageTypeFlagsEXT type,
                                 const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                 void* pUserData) {
	std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

	return vk::False;
}

bool Device::checkDeviceSuitable(const vk::raii::PhysicalDevice& phyDevice) {
	// Check if the physicalDevice supports the Vulkan 1.3 API version
	bool supportsVulkan1_3 = phyDevice.getProperties().apiVersion >= vk::ApiVersion13;

	// Check if any of the queue families support graphics operations
	bool supportsGraphics = std::ranges::any_of(phyDevice.getQueueFamilyProperties(), [&](const auto& qfp) {
		return static_cast<bool>(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
	});

	// Check if all required physicalDevice extensions are available
	std::unordered_set<std::string_view> availableSet;
	for (const auto& [extensionName, specVersion]: phyDevice.enumerateDeviceExtensionProperties()) {
		availableSet.insert(extensionName);
	}

	bool supportsAllRequiredExtensions = std::ranges::all_of(deviceExtensions, [&](const char* required) {
		return availableSet.contains(required);
	});

	// Check if the physicalDevice supports the required features (dynamic rendering and extended dynamic state)
	auto features2 = phyDevice.getFeatures2<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

	// Perform the checks with clear boolean logic
	bool supportsSamplerAnisotropy = features2.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy;
	bool supportsShaderDrawParameters = features2.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters;
	bool supportsDynamicRendering = features2.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering;
	bool supportsSynchronization2 = features2.get<vk::PhysicalDeviceVulkan13Features>().synchronization2;
	bool supportsExtendedDynamicState = features2.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().
			extendedDynamicState;
	bool supportsRequiredFeatures =
			supportsSamplerAnisotropy &&
			supportsShaderDrawParameters &&
			supportsDynamicRendering &&
			supportsSynchronization2 &&
			supportsExtendedDynamicState;

	return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

void Device::copyBuffer(const Buffer& dstBuffer, const Buffer& srcBuffer, const vk::DeviceSize size) const {
	const vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
	commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
	endSingleTimeCommands(commandCopyBuffer);
}

vk::raii::CommandBuffer Device::beginSingleTimeCommands() const {
	const vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = **mCommandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	vk::raii::CommandBuffer commandBuffer = std::move(mDevice.allocateCommandBuffers(allocInfo).front());
	constexpr vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
	commandBuffer.begin(beginInfo);

	return commandBuffer;
}

void Device::endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const {
	commandBuffer.end();

	const vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandBuffer};

	mQueue.submit(submitInfo, nullptr);
	mQueue.waitIdle();
}
