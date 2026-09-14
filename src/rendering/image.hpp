#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include "memory.hpp"

class CommandBuffer;

class Image {
public:
	Image() = default;

	Image(const vk::raii::Device& device,
	      const vk::raii::PhysicalDevice& phyDev,
	      uint32_t width,
	      uint32_t height,
	      uint32_t mipLevels,
	      vk::SampleCountFlagBits numSamples,
	      vk::Format format,
	      vk::ImageTiling tiling,
	      vk::ImageUsageFlags usage,
	      vk::MemoryPropertyFlags properties);

	[[nodiscard]]
	const vk::raii::ImageView& view() const noexcept;

	vk::raii::Image& operator*() noexcept { return mImage; }
	const vk::raii::Image& operator*() const noexcept { return mImage; }

	static void transitionImageLayout(vk::Image image,
	                                  vk::PipelineStageFlags2 srcStageMask,
	                                  vk::AccessFlags2 srcAccessMask,
	                                  vk::PipelineStageFlags2 dstStageMask,
	                                  vk::AccessFlags2 dstAccessMask,
	                                  vk::ImageLayout oldLayout,
	                                  vk::ImageLayout newLayout,
	                                  const CommandBuffer& commandBuffer);

private:
	vk::raii::Image mImage{nullptr};
	DeviceMemory mImageMemory;
	vk::raii::ImageView mImageView{nullptr};
};
