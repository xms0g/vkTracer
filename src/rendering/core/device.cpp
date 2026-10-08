#include "device.hpp"
#include <set>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <SDL_vulkan.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "image/stb_image.h"
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
#include "../quad.hpp"
#include "../scene.hpp"
#include "../volume.hpp"
#include "../../core/window.hpp"
#include "../../core/camera.hpp"
#include "../../io/filesystem.hpp"

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
		createTextureImage(TEXTURE_PATH);
		createSamplers();
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

	vk::ValidationFeaturesEXT syncValidationFeature = {};
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

		syncValidationFeature.enabledValidationFeatureCount = static_cast<uint32_t>(syncValidationFeatures.size());
		syncValidationFeature.pEnabledValidationFeatures = syncValidationFeatures.data();
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
		.pNext = &syncValidationFeature,
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
			.addBinding(
				2,
				vk::DescriptorType::eCombinedImageSampler,
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
			.addMaxSets(MAX_FRAMES_IN_FLIGHT * 3)
			.addPoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
			.addPoolSize(vk::DescriptorType::eStorageImage, MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT * 2)
			.build();
}

void Device::createShaderStorageBuffers() {
	std::vector<GPUSphere> gpuSpheres;
	std::vector<GPUQuad> gpuQuads;
	std::vector<GPUVolume> gpuVolumes;

	const auto& [bvh, sphereCount, quadCount, volumeCount, bvhNodeCount] = Scene::buildScene();
	const vk::DeviceSize bvhBufferSize = sizeof(GPUBVHNode) * bvhNodeCount;
	const vk::DeviceSize sphereBufferSize = sizeof(GPUSphere) * sphereCount;
	const vk::DeviceSize quadBufferSize = sizeof(GPUQuad) * quadCount;
	const vk::DeviceSize volumeBufferSize = sizeof(GPUVolume) * volumeCount;

	const std::array bufferSizes = {sphereBufferSize, quadBufferSize, volumeBufferSize, bvhBufferSize};
	std::array<Buffer, bufferSizes.size()> stagingBuffers;

	for (uint32_t i = 0; i < stagingBuffers.size(); ++i) {
		stagingBuffers[i] =
				Buffer{
					bufferSizes[i],
					mDevice,
					mPhysicalDevice,
					vk::BufferUsageFlagBits::eTransferSrc,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
				};
	}

	mShaderStorageBuffers.clear();

	// Copy initial sphere data to all storage buffers
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		gpuSpheres.clear();
		gpuQuads.clear();
		gpuVolumes.clear();

		for (auto bufferSize: bufferSizes) {
			mShaderStorageBuffers.emplace_back(
				bufferSize,
				mDevice,
				mPhysicalDevice,
				vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst |
				vk::BufferUsageFlagBits::eShaderDeviceAddress,
				vk::MemoryPropertyFlagBits::eDeviceLocal,
				vk::MemoryAllocateFlagsInfo{.flags = vk::MemoryAllocateFlagBits::eDeviceAddress});
		}

		Buffer& spheresSSBO = mShaderStorageBuffers[0];
		Buffer& quadSSBO = mShaderStorageBuffers[1];
		Buffer& volumeSSBO = mShaderStorageBuffers[2];
		Buffer& bvhSSBO = mShaderStorageBuffers[3];

		auto getBufferAddress = [&](Buffer& buffer) -> uint64_t {
			const vk::BufferDeviceAddressInfo info{
				.buffer = **buffer
			};

			return mDevice.getBufferAddress(info);
		};

		const uint64_t sphereAddress = getBufferAddress(spheresSSBO);
		const uint64_t quadAddress = getBufferAddress(quadSSBO);
		const uint64_t volumeAddress = getBufferAddress(volumeSSBO);
		const uint64_t bvhAddress = getBufferAddress(bvhSSBO);

		const auto gpuBVH = BVHNode::flatten(
			*bvh,
			gpuSpheres,
			gpuQuads,
			gpuVolumes,
			bvhAddress,
			sphereAddress,
			quadAddress,
			volumeAddress);

		Buffer& sphereStagingBuffer = stagingBuffers[0];
		Buffer& quadStagingBuffer = stagingBuffers[1];
		Buffer& volumeStagingBuffer = stagingBuffers[2];
		Buffer& bvhStagingBuffer = stagingBuffers[3];

		auto copyDataToBuffer = [&](const void* data,
		                            Buffer& stagingBuffer,
		                            const Buffer& ssbo,
		                            const size_t bufferSize) {
			void* mem = stagingBuffer.map(bufferSize);
			memcpy(mem, data, bufferSize);
			stagingBuffer.unmap();

			copyBuffer(stagingBuffer, ssbo, bufferSize);
		};

		copyDataToBuffer(gpuSpheres.data(), sphereStagingBuffer, spheresSSBO, sphereBufferSize);
		copyDataToBuffer(gpuQuads.data(), quadStagingBuffer, quadSSBO, quadBufferSize);
		copyDataToBuffer(gpuVolumes.data(), volumeStagingBuffer, volumeSSBO, volumeBufferSize);
		copyDataToBuffer(gpuBVH.data(), bvhStagingBuffer, bvhSSBO, bvhBufferSize);

		mShaderStorageBufferAddresses[i] = bvhAddress;
	}
}

void Device::createShaderStorageImage() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		mShaderStorageImages[i] = Image(
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

void Device::createTextureImage(const std::string_view path) {
	int32_t texWidth, texHeight, texChannels;
	void* pixels = stbi_load(fs::path(path.data()).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	const vk::DeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("Failed to load texture image!");
	}

	Buffer stagingBuffer{
		imageSize,
		mDevice,
		mPhysicalDevice,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	void* mem = stagingBuffer.map(imageSize);
	memcpy(mem, pixels, imageSize);
	stagingBuffer.unmap();

	stbi_image_free(pixels);

	mTextureImages.emplace_back(
		mDevice,
		mPhysicalDevice,
		ImageConfig{
			.width = static_cast<uint32_t>(texWidth),
			.height = static_cast<uint32_t>(texHeight),
			.mipLevels = 1,
			.numSamples = vk::SampleCountFlagBits::e1,
			.format = vk::Format::eR8G8B8A8Srgb,
			.tiling = vk::ImageTiling::eOptimal,
			.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			.properties = vk::MemoryPropertyFlagBits::eDeviceLocal
		});

	copyBufferToImage(stagingBuffer, mTextureImages.back(), static_cast<uint32_t>(texWidth),
	                  static_cast<uint32_t>(texHeight));
}

void Device::createSamplers() {
	const vk::PhysicalDeviceProperties properties = mPhysicalDevice.getProperties();
	const vk::SamplerCreateInfo textureSamplerInfo{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.0f,
		.anisotropyEnable = vk::True,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.0f,
		.maxLod = vk::LodClampNone
	};

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

	mSamplers.emplace_back(mDevice, samplerInfo);
	mSamplers.emplace_back(mDevice, textureSamplerInfo);
}

void Device::createDescriptorSets() {
	const DescriptorSetAllocator allocator(mDevice, mDescriptorPool);
	mComputeDescriptorSets = allocator.allocate(MAX_FRAMES_IN_FLIGHT, **mComputeDescriptorSetLayout);
	mGraphicsDescriptorSets = allocator.allocate(MAX_FRAMES_IN_FLIGHT, **mGraphicsDescriptorSetLayout);

	DescriptorSetWriter writer(mDevice);
	writer.reserve(MAX_FRAMES_IN_FLIGHT * 3);

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
					mSamplers.front())
				.writeImage(
					*mComputeDescriptorSets[i],
					2,
					vk::DescriptorType::eCombinedImageSampler,
					mTextureImages.back().view(),
					vk::ImageLayout::eShaderReadOnlyOptimal,
					mSamplers.back());

		writer.update();
	}
}

void Device::createCommandBuffers() {
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		mGraphicsCommandBuffers[i] = CommandBuffer(mDevice, mCommandPool, vk::CommandBufferLevel::ePrimary);
		mComputeCommandBuffers[i] = CommandBuffer(mDevice, mCommandPool, vk::CommandBufferLevel::ePrimary);
	}
}

void Device::recordGraphicsCommandBuffer(const uint32_t imageIndex) {
	const auto& cmd = mGraphicsCommandBuffers[mFrameIndex];
	(*cmd).reset();
	(*cmd).begin({});

	const auto& image = mSwapchain.image(imageIndex);
	// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
	Image::transitionImageLayout(
		image,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		cmd
	);

	vk::RenderingAttachmentInfo attachmentInfo = {
		.imageView = mSwapchain.imageView(imageIndex),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)
	};

	const vk::RenderingInfo renderingInfo = {
		.renderArea = {.offset = {.x = 0, .y = 0}, .extent = mSwapchain.extent()},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentInfo
	};

	(*cmd).beginRendering(renderingInfo);
	(*cmd).bindPipeline(vk::PipelineBindPoint::eGraphics, **mGraphicsPipeline);
	(*cmd).bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		mGraphicsPipeline.layout(),
		0,
		{mGraphicsDescriptorSets[mFrameIndex]},
		{});
	(*cmd).setViewport(
		0,
		vk::Viewport(
			0.0f,
			0.0f,
			static_cast<float>(mSwapchain.extent().width),
			static_cast<float>(mSwapchain.extent().height),
			0.0f,
			1.0f));
	(*cmd).setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), mSwapchain.extent()));
	(*cmd).draw(3, 1, 0, 0);
	(*cmd).endRendering();
	// After rendering, transition the swapchain image to PRESENT_SRC
	Image::transitionImageLayout(
		image,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		cmd
	);
	(*cmd).end();
}

void Device::recordComputeCommandBuffer() {
	const auto& cmd = mComputeCommandBuffers[mFrameIndex];
	(*cmd).reset();
	(*cmd).begin({});

	if (!mShaderStorageImageInitialized[mFrameIndex]) {
		Image::transitionImageLayout(
			**mShaderStorageImages[mFrameIndex],
			vk::PipelineStageFlagBits2::eTopOfPipe,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderWrite,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eGeneral,
			cmd
		);

		mShaderStorageImageInitialized[mFrameIndex] = true;
	} else {
		Image::transitionImageLayout(
			**mShaderStorageImages[mFrameIndex],
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderSampledRead,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderWrite,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eGeneral,
			cmd
		);
	}

	(*cmd).bindPipeline(vk::PipelineBindPoint::eCompute, **mComputePipeline);
	(*cmd).bindDescriptorSets(
		vk::PipelineBindPoint::eCompute,
		mComputePipeline.layout(),
		0,
		{mComputeDescriptorSets[mFrameIndex]}, {});

	(*cmd).pushConstants<ComputePushConstants>(
		mComputePipeline.layout(),
		vk::ShaderStageFlagBits::eCompute,
		0,
		ComputePushConstants{
			.bufferAddress = mShaderStorageBufferAddresses[mFrameIndex],
			.currentTile = mCurrentTiles[mFrameIndex],
			.resolution = glm::vec4(WIDTH, HEIGHT, 0, 0),
			.camCenter = glm::vec4(mCamera.center(), 0.0f),
			.camFront = glm::vec4(mCamera.front(), 0.0f),
			.camRight = glm::vec4(mCamera.right(), 0.0f),
			.camUp = glm::vec4(mCamera.up(), 0.0f),
		});

	(*cmd).dispatch(TILES_PER_FRAME, 1, 1);

	Image::transitionImageLayout(
		**mShaderStorageImages[mFrameIndex],
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eGeneral,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		cmd
	);

	(*cmd).end();
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
	const auto cmd = beginSingleTimeCommands();
	(*cmd).copyBuffer(
		**srcBuffer,
		**dstBuffer,
		vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = size});
	endSingleTimeCommands(cmd);
}

void Device::copyBufferToImage(const Buffer& srcBuffer,
                               const Image& dstImage,
                               const uint32_t width,
                               const uint32_t height) const {
	const auto cmd = beginSingleTimeCommands();

	Image::transitionImageLayout(
		**dstImage,
		vk::PipelineStageFlagBits2::eTopOfPipe,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eTransfer,
		vk::AccessFlagBits2::eTransferWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal,
		cmd
	);

	(*cmd).copyBufferToImage(
		**srcBuffer,
		**dstImage,
		vk::ImageLayout::eTransferDstOptimal,
		vk::BufferImageCopy{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.imageOffset = {.x = 0, .y = 0, .z = 0},
			.imageExtent = {.width = width, .height = height, .depth = 1}
		});

	Image::transitionImageLayout(
		**dstImage,
		vk::PipelineStageFlagBits2::eTransfer,
		vk::AccessFlagBits2::eTransferWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		cmd
	);

	endSingleTimeCommands(cmd);
}

CommandBuffer Device::beginSingleTimeCommands() const {
	auto commandBuffer = CommandBuffer(mDevice, mCommandPool, vk::CommandBufferLevel::ePrimary);

	constexpr vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
	(*commandBuffer).begin(beginInfo);

	return commandBuffer;
}

void Device::endSingleTimeCommands(const CommandBuffer& commandBuffer) const {
	(*commandBuffer).end();

	const vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &**commandBuffer};

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
	bool supportsExtendedDynamicState = features2.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().
			extendedDynamicState;
	bool supportsRequiredFeatures =
			supportsSamplerAnisotropy &&
			supportsShaderDrawParameters &&
			supportBufferDeviceAddress &&
			supportsDynamicRendering &&
			supportsSynchronization2 &&
			supportsExtendedDynamicState;

	return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}
