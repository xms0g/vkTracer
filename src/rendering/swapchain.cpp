#include "swapchain.h"

Swapchain::Swapchain(
	const vk::raii::SurfaceKHR& surface,
	const vk::raii::Device& device,
	const vk::raii::PhysicalDevice& phyDev,
	GLFWwindow& window) {
	create(surface, device, phyDev, window);
}

vk::SurfaceFormatKHR& Swapchain::surfaceFormat() {
	return mSwapChainSurfaceFormat;
}

vk::Image& Swapchain::image(const uint32_t imageIndex) {
	return mSwapChainImages[imageIndex];
}

vk::raii::ImageView& Swapchain::imageView(const uint32_t imageIndex) {
	return mSwapChainImageViews[imageIndex];
}

vk::Extent2D& Swapchain::extent() {
	return mSwapChainExtent;
}

uint32_t Swapchain::acquireNextImage(const vk::raii::Fence& fence) const {
	auto [result, imageIndex] = mSwapChain.acquireNextImage(UINT64_MAX,nullptr, fence);
	if (result != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	return imageIndex;
}

void Swapchain::recreate(
	const vk::raii::SurfaceKHR& surface,
	const vk::raii::Device& device,
	const vk::raii::PhysicalDevice& phyDev,
	GLFWwindow& window) {
	device.waitIdle();

	mSwapChainImageViews.clear();
	mSwapChain = nullptr;

	create(surface, device, phyDev, window);
	createSwapchainImageViews(device);
}

void Swapchain::create(
	const vk::raii::SurfaceKHR& surface,
	const vk::raii::Device& device,
	const vk::raii::PhysicalDevice& phyDev,
	GLFWwindow& window) {
	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = phyDev.getSurfaceCapabilitiesKHR(surface);
	mSwapChainExtent = chooseSwapExtent(surfaceCapabilities, window);
	const uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

	const std::vector<vk::SurfaceFormatKHR> availableFormats = phyDev.getSurfaceFormatsKHR(surface);
	const vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(availableFormats);
	mSwapChainSurfaceFormat = surfaceFormat;

	const std::vector<vk::PresentModeKHR> availablePresentModes = phyDev.getSurfacePresentModesKHR(surface);
	const vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{
		.surface = surface,
		.minImageCount = minImageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = mSwapChainExtent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = true
	};

	swapChainCreateInfo.oldSwapchain = nullptr;

	mSwapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
	mSwapChainImages = mSwapChain.getImages();

	createSwapchainImageViews(device);
}

void Swapchain::createSwapchainImageViews(const vk::raii::Device& device) {
	assert(mSwapChainImageViews.empty());

	vk::ImageViewCreateInfo imageViewCreateInfo{
		.viewType = vk::ImageViewType::e2D,
		.format = mSwapChainSurfaceFormat.format,
		.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
	};

	for (const auto& image: mSwapChainImages) {
		imageViewCreateInfo.image = image;
		mSwapChainImageViews.emplace_back(device, imageViewCreateInfo);
	}
}

vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
	const auto formatIt = std::ranges::find_if(availableFormats, [](const auto& format) {
		return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
	});

	if (formatIt != availableFormats.end()) {
		return *formatIt;
	}

	return availableFormats[0];
}

vk::PresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
	const auto modeIt = std::ranges::find_if(
		availablePresentModes, [](const auto& mode) { return mode == vk::PresentModeKHR::eMailbox; });

	if (modeIt != availablePresentModes.end()) {
		return *modeIt;
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow& window) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	int width, height;
	glfwGetFramebufferSize(&window, &width, &height);

	return {
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

uint32_t Swapchain::chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) {
	auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);

	if ((surfaceCapabilities.maxImageCount > 0) && (surfaceCapabilities.maxImageCount < minImageCount)) {
		minImageCount = surfaceCapabilities.maxImageCount;
	}

	return minImageCount;
}
