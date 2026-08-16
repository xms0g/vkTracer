#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include "vertex.hpp"
#include "shader.h"

class DescriptorSetLayout;
class Swapchain;
class Shader;

class PipelineBuilder {
public:
	explicit PipelineBuilder(const vk::raii::Device& device);

	void reset();

	PipelineBuilder& addVertexShader(Shader& shader, const std::string& entry);

	PipelineBuilder& addFragmentShader(Shader& shader, const std::string& entry);

	PipelineBuilder& addComputeShader(Shader& shader, const std::string& entry);

	PipelineBuilder& vertexInput(const VertexLayout& layout);

	PipelineBuilder& topology(vk::PrimitiveTopology topology);

	PipelineBuilder& viewportState(uint32_t viewportCount, uint32_t scissorCount);

	template<vk::DynamicState T>
	PipelineBuilder& dynamicState();

	template<vk::DynamicState... States>
	PipelineBuilder& dynamicStates();

	PipelineBuilder& rasterizer();

	PipelineBuilder& multisampling();

	PipelineBuilder& alphaBlending();

	vk::raii::PipelineLayout createPipelineLayout(
		const vk::DescriptorSetLayout* dscSetLayout = nullptr,
		uint32_t dscSetLayoutCount = 0,
		uint32_t pushConstantSize = 0,
		vk::ShaderStageFlags stages = {}) const;

	vk::raii::Pipeline buildGraphics(vk::SurfaceFormatKHR& surfaceFormat, const vk::raii::PipelineLayout& layout);

	[[nodiscard]]
	vk::raii::Pipeline buildCompute(const vk::raii::PipelineLayout& layout) const;

private:
	vk::PipelineInputAssemblyStateCreateInfo mInputAssembly;
	vk::PipelineViewportStateCreateInfo mViewportState;
	vk::PipelineRasterizationStateCreateInfo mRasterizer;
	vk::PipelineMultisampleStateCreateInfo mMultisampling;
	vk::PipelineColorBlendAttachmentState mColorBlendAttachment;
	vk::PipelineColorBlendStateCreateInfo mColorBlending;
	vk::PipelineDynamicStateCreateInfo mDynamicState;
	vk::PipelineVertexInputStateCreateInfo mVertexInputInfo;
	std::vector<vk::PipelineShaderStageCreateInfo> mShaderStages;
	std::vector<vk::DynamicState> mDynamicStates;
	const vk::raii::Device& mDevice;
};

template<vk::DynamicState T>
PipelineBuilder& PipelineBuilder::dynamicState() {
	mDynamicStates.push_back(T);
	return *this;
}

template<vk::DynamicState... States>
PipelineBuilder& PipelineBuilder::dynamicStates() {
	(mDynamicStates.push_back(States), ...);
	return *this;
}

class Pipeline {
public:
	Pipeline() = default;

	vk::raii::PipelineLayout& layout() { return mPipelineLayout; }

	vk::raii::Pipeline& operator*() noexcept { return mPipeline; }
	const vk::raii::Pipeline& operator*() const noexcept { return mPipeline; }

protected:
	vk::raii::Pipeline mPipeline{nullptr};
	vk::raii::PipelineLayout mPipelineLayout{nullptr};
};

class GraphicsPipeline : public Pipeline {
public:
	GraphicsPipeline() = default;

	GraphicsPipeline(PipelineBuilder& builder, Shader& shader, vk::SurfaceFormatKHR& surfaceFormat, const VertexLayout& layout);

private:
	VertexLayout mVertexLayout;
};

class ComputePipeline : public Pipeline {
public:
	ComputePipeline() = default;

	ComputePipeline(
		PipelineBuilder& builder,
		Shader& shader,
		DescriptorSetLayout& dscSetLayout,
		uint32_t dscSetLayoutCount,
		uint32_t pushConstantSize);
};
