#include "image.hpp"
#include "commandBuffer.hpp"

Image::Image(const vk::raii::Device& device,
             const vk::raii::PhysicalDevice& phyDev,
             const uint32_t width,
             const uint32_t height,
             const uint32_t mipLevels,
             const vk::SampleCountFlagBits numSamples,
             const vk::Format format,
             const vk::ImageTiling tiling,
             const vk::ImageUsageFlags usage,
             const vk::MemoryPropertyFlags properties) {
	const vk::ImageCreateInfo imageInfo{
		.imageType = vk::ImageType::e2D,
		.format = format,
		.extent = vk::Extent3D{
			.width = width,
			.height = height,
			.depth = 1
		},
		.mipLevels = mipLevels,
		.arrayLayers = 1,
		.samples = numSamples,
		.tiling = tiling,
		.usage = usage,
		.initialLayout = vk::ImageLayout::eUndefined
	};

	mImage = vk::raii::Image(device, imageInfo);

	const vk::MemoryRequirements memRequirements = mImage.getMemoryRequirements();
	mImageMemory = DeviceMemory(device, phyDev, memRequirements.size, memRequirements.memoryTypeBits, properties);

	mImage.bindMemory(*mImageMemory, 0);

	const vk::ImageViewCreateInfo viewInfo{
		.image = mImage,
		.viewType = vk::ImageViewType::e2D,
		.format = format,

		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	mImageView = vk::raii::ImageView(device, viewInfo);
}

const vk::raii::ImageView& Image::view() const noexcept {
	return mImageView;
}

void Image::transitionImageLayout(const vk::Image image,
                                  const vk::PipelineStageFlags2 srcStageMask,
                                  const vk::AccessFlags2 srcAccessMask,
                                  const vk::PipelineStageFlags2 dstStageMask,
                                  const vk::AccessFlags2 dstAccessMask,
                                  const vk::ImageLayout oldLayout,
                                  const vk::ImageLayout newLayout,
                                  const CommandBuffer& commandBuffer) {
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = srcStageMask,
		.srcAccessMask = srcAccessMask,
		.dstStageMask = dstStageMask,
		.dstAccessMask = dstAccessMask,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	const vk::DependencyInfo dependencyInfo = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};

	(*commandBuffer).pipelineBarrier2(dependencyInfo);
}
