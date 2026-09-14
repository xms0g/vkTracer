#include "image.hpp"
#include "commandBuffer.hpp"

Image::Image(const vk::raii::Device& device,
             const vk::raii::PhysicalDevice& phyDev,
             const ImageConfig& config) {
	const vk::ImageCreateInfo imageInfo{
		.imageType = vk::ImageType::e2D,
		.format = config.format,
		.extent = vk::Extent3D{
			.width = config.width,
			.height = config.height,
			.depth = 1
		},
		.mipLevels = config.mipLevels,
		.arrayLayers = 1,
		.samples = config.numSamples,
		.tiling = config.tiling,
		.usage = config.usage,
		.initialLayout = vk::ImageLayout::eUndefined
	};

	mImage = vk::raii::Image(device, imageInfo);

	const vk::MemoryRequirements memRequirements = mImage.getMemoryRequirements();
	mImageMemory = DeviceMemory(device, phyDev, memRequirements.size, memRequirements.memoryTypeBits,
	                            config.properties);

	mImage.bindMemory(*mImageMemory, 0);

	const vk::ImageViewCreateInfo viewInfo{
		.image = mImage,
		.viewType = vk::ImageViewType::e2D,
		.format = config.format,

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

Image::Image(Image&& other) noexcept
	: mImage(std::move(other.mImage)),
	  mImageView(std::move(other.mImageView)),
	  mImageMemory(std::move(other.mImageMemory)) {
}

Image& Image::operator=(Image&& other) noexcept {
	if (this != &other) {
		mImage = std::move(other.mImage);
		mImageView = std::move(other.mImageView);
		mImageMemory = std::move(other.mImageMemory);
	}
	return *this;
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