#include "device.hpp"
#include <set>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <random>
#include <unordered_set>
#include <SDL_vulkan.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "buffer.hpp"
#include "swapchain.hpp"
#include "commandPool.hpp"
#include "descriptorPool.hpp"
#include "descriptorSet.hpp"
#include "deviceExtension.hpp"
#include "image.hpp"
#include "pipelineBuilder.hpp"
#include "validation.hpp"
#include "../bvh.hpp"
#include "../sphere.hpp"
#include "../../core/window.hpp"
#include "../../config/config.hpp"
#include "../../core/camera.hpp"

Device::Device(Window& window, Camera& camera)
	: mWindow(window),
	  mCamera(camera) {
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
		createShaderStorageBuffers();
		createSampler();
		createDescriptorPool();
		createDescriptorSets();
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
	if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR) {
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

	const auto sdlExtensions = getRequiredInstanceExtensions();

	std::unordered_set<std::string> supportedExtensions;
	for (const auto& [extensionName, specVersion]: mContext.enumerateInstanceExtensionProperties()) {
		supportedExtensions.insert(extensionName);
	}

	std::vector<const char*> requiredExtensions;
	for (const auto& extension: sdlExtensions) {
		if (!supportedExtensions.contains(extension)) {
			throw std::runtime_error("Required SDL extension not supported: " + std::string(extension));
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

	if (!SDL_Vulkan_CreateSurface(&*mWindow, *mInstance, &surface)) {
		throw std::runtime_error(std::format("SDL_Vulkan_CreateSurface failed: {}", SDL_GetError()));
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
		vk::PhysicalDeviceVulkan12Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
		{.features = {.samplerAnisotropy = true, .shaderInt64 = true},},
		{.shaderDrawParameters = true},
		{.timelineSemaphore = true, .bufferDeviceAddress = true},
		{.synchronization2 = true, .dynamicRendering = true},
		{.extendedDynamicState = true}
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
				1,
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
		1,
		0);

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

void Device::createShaderStorageBuffers() {
	std::vector<GPUSphere> gpuSpheres;
	auto spheres = Sphere::generateSpheres();
	const auto bvh = BVHNode(spheres, 0, spheres.size());

	vk::DeviceSize bvhBufferSize = sizeof(GPUBVHNode) * BVHNode::count;
	vk::DeviceSize sphereBufferSize = sizeof(GPUSphere) * spheres.size();

	// Create a staging buffer used to upload data to the gpu
	Buffer sphereStagingBuffer{
		sphereBufferSize,
		mDevice,
		mPhysicalDevice,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};
	Buffer bvhStagingBuffer{
		bvhBufferSize,
		mDevice,
		mPhysicalDevice,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	mShaderStorageBuffers.clear();

	// Copy initial sphere data to all storage buffers
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		gpuSpheres.clear();

		Buffer spheresSSBO{
			sphereBufferSize,
			mDevice,
			mPhysicalDevice,
			vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst |
			vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::MemoryAllocateFlagsInfo{.flags = vk::MemoryAllocateFlagBits::eDeviceAddress}};

		Buffer bvhSSBO{
			bvhBufferSize,
			mDevice,
			mPhysicalDevice,
			vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst |
			vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::MemoryAllocateFlagsInfo{.flags = vk::MemoryAllocateFlagBits::eDeviceAddress}};

		vk::BufferDeviceAddressInfo info{
			.buffer = **spheresSSBO
		};
		uint64_t sphereAddress = mDevice.getBufferAddress(info);
		info.buffer = **bvhSSBO;
		uint64_t bvhAddress = mDevice.getBufferAddress(info);

		const auto gpuBVH = BVHNode::flatten(bvh, gpuSpheres, bvhAddress, sphereAddress);

		void* mem = sphereStagingBuffer.map(sphereBufferSize);
		memcpy(mem, gpuSpheres.data(), sphereBufferSize);
		sphereStagingBuffer.unmap();

		copyBuffer(sphereStagingBuffer, spheresSSBO, sphereBufferSize);

		mem = bvhStagingBuffer.map(bvhBufferSize);
		memcpy(mem, gpuBVH.data(), bvhBufferSize);
		bvhStagingBuffer.unmap();

		copyBuffer(bvhStagingBuffer, bvhSSBO, bvhBufferSize);

		mShaderStorageBuffers.emplace_back(std::move(spheresSSBO));
		mShaderStorageBuffers.emplace_back(std::move(bvhSSBO));

		mShaderStorageBufferAddresses.push_back(bvhAddress);
	}
}

void Device::createShaderStorageImage() {
	mShaderStorageImages.clear();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		mShaderStorageImages.emplace_back(
			mDevice,
			mPhysicalDevice,
			ImageConfig{
				.width = WIDTH,
				.height = HEIGHT,
				.mipLevels = 1,
				.numSamples = vk::SampleCountFlagBits::e1,
				.format = vk::Format::eR8G8B8A8Unorm,
				.tiling = vk::ImageTiling::eOptimal,
				.usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
				.properties = vk::MemoryPropertyFlagBits::eDeviceLocal
			});
	}
}

void Device::createSampler() {
	constexpr vk::SamplerCreateInfo samplerInfo{
		.magFilter = vk::Filter::eNearest,
		.minFilter = vk::Filter::eNearest,
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

	mSampler = vk::raii::Sampler(mDevice, samplerInfo);
}

void Device::createDescriptorSets() {
	const DescriptorSetAllocator allocator(mDevice, mDescriptorPool);
	mComputeDescriptorSets = allocator.allocate(MAX_FRAMES_IN_FLIGHT, **mComputeDescriptorSetLayout);
	mGraphicsDescriptorSets = allocator.allocate(MAX_FRAMES_IN_FLIGHT, **mGraphicsDescriptorSetLayout);

	DescriptorSetWriter writer(mDevice);
	writer.reserve(MAX_FRAMES_IN_FLIGHT * 2);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		writer.writeImage(
					*mComputeDescriptorSets[i],
					0,
					vk::DescriptorType::eStorageImage,
					mShaderStorageImages[i].view(),
					vk::ImageLayout::eGeneral)
				.writeImage(
					*mGraphicsDescriptorSets[i],
					1,
					vk::DescriptorType::eCombinedImageSampler,
					mShaderStorageImages[i].view(),
					vk::ImageLayout::eShaderReadOnlyOptimal,
					mSampler);

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
	Image::transitionImageLayout(
		image,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		{},
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
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
		.renderArea = {.offset = {.x = 0, .y = 0}, .extent = mSwapchain.extent()},
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
	Image::transitionImageLayout(
		image,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eBottomOfPipe,
		{},
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		commandBuffer
	);
	(*commandBuffer).end();
}

void Device::recordComputeCommandBuffer() {
	const auto& commandBuffer = mComputeCommandBuffers[mFrameIndex];
	(*commandBuffer).reset();
	(*commandBuffer).begin({});

	Image::transitionImageLayout(
		**mShaderStorageImages[mFrameIndex],
		vk::PipelineStageFlagBits2::eTopOfPipe,
		{},
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eGeneral,
		commandBuffer
	);

	(*commandBuffer).bindPipeline(vk::PipelineBindPoint::eCompute, **mComputePipeline);
	(*commandBuffer).bindDescriptorSets(
		vk::PipelineBindPoint::eCompute,
		mComputePipeline.layout(),
		0,
		{mComputeDescriptorSets[mFrameIndex]}, {});

	const ComputePushConstants pc{
		.bufferAddress = mShaderStorageBufferAddresses[mFrameIndex],
		.resolution = glm::vec4(WIDTH, HEIGHT, 0, 0),
		.camCenter = glm::vec4(mCamera.center(), 0.0f),
		.camFront = glm::vec4(mCamera.front(), 0.0f),
		.camRight = glm::vec4(mCamera.right(), 0.0f),
		.camUp = glm::vec4(mCamera.up(), 0.0f),
	};

	(*commandBuffer).pushConstants(
		mComputePipeline.layout(),
		vk::ShaderStageFlagBits::eCompute,
		0,
		vk::ArrayProxy<const ComputePushConstants>(pc));

	(*commandBuffer).dispatch(
		WIDTH / THREADS_PER_GROUP,
		HEIGHT / THREADS_PER_GROUP,
		1);

	Image::transitionImageLayout(
		**mShaderStorageImages[mFrameIndex],
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eGeneral,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		commandBuffer
	);

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

void Device::copyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, const vk::DeviceSize size) const {
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

std::vector<const char*> Device::getRequiredInstanceExtensions() const {
	uint32_t extensionCount = 0;

	if (!SDL_Vulkan_GetInstanceExtensions(&*mWindow, &extensionCount, nullptr)) {
		throw std::runtime_error(SDL_GetError());
	}

	std::vector<const char*> extensions(extensionCount);

	if (!SDL_Vulkan_GetInstanceExtensions(&*mWindow, &extensionCount, extensions.data())) {
		throw std::runtime_error(SDL_GetError());
	}

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
		vk::PhysicalDeviceVulkan12Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

	// Perform the checks with clear boolean logic
	bool supportsSamplerAnisotropy = features2.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy;
	bool supportsShaderDrawParameters = features2.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters;
	bool supportBufferDeviceAddress = features2.get<vk::PhysicalDeviceVulkan12Features>().bufferDeviceAddress;
	bool supportsDynamicRendering = features2.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering;
	bool supportsSynchronization2 = features2.get<vk::PhysicalDeviceVulkan13Features>().synchronization2;
	bool supportsExtendedDynamicState = features2.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
	bool supportsRequiredFeatures =
			supportsSamplerAnisotropy &&
			supportsShaderDrawParameters &&
			supportBufferDeviceAddress &&
			supportsDynamicRendering &&
			supportsSynchronization2 &&
			supportsExtendedDynamicState;

	return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}
