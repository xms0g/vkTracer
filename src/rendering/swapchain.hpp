#pragma once
#include <SDL.h>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Window;
class Swapchain {
public:
	Swapchain() = default;

	Swapchain(const vk::raii::SurfaceKHR& surface,
	          const vk::raii::Device& device,
	          const vk::raii::PhysicalDevice& phyDev,
	          SDL_Window& window);

	vk::SurfaceFormatKHR& surfaceFormat();

	vk::Image& image(uint32_t imageIndex);

	vk::raii::ImageView& imageView(uint32_t imageIndex);

	vk::Extent2D& extent();

	[[nodiscard]]
	uint32_t acquireNextImage(const vk::raii::Fence& fence) const;

	void recreate(const vk::raii::SurfaceKHR& surface,
	              const vk::raii::Device& device,
	              const vk::raii::PhysicalDevice& phyDev,
	              SDL_Window& window);

	vk::raii::SwapchainKHR& operator*() noexcept { return mSwapChain; }
	const vk::raii::SwapchainKHR& operator*() const noexcept { return mSwapChain; }

private:
	void create(const vk::raii::SurfaceKHR& surface,
	            const vk::raii::Device& device,
	            const vk::raii::PhysicalDevice& phyDev,
	            SDL_Window& window);

	void createSwapchainImageViews(const vk::raii::Device& device);

	static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

	static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

	static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, SDL_Window& window);

	static uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities);

	vk::raii::SwapchainKHR mSwapChain{nullptr};
	vk::SurfaceFormatKHR mSwapChainSurfaceFormat;
	vk::Extent2D mSwapChainExtent;
	std::vector<vk::Image> mSwapChainImages;
	std::vector<vk::raii::ImageView> mSwapChainImageViews;
};
