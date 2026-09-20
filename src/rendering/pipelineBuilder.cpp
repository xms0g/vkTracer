#include "pipelineBuilder.hpp"
#include "swapchain.hpp"
#include "descriptorSetLayout.hpp"

PipelineBuilder::PipelineBuilder(const vk::raii::Device& device)
	: mDevice(device) {
}

void PipelineBuilder::reset() {
	mShaderStages.clear();
	mDynamicStates.clear();
	mEntryPointNames.clear();
}

PipelineBuilder& PipelineBuilder::addVertexShader(Shader& shader, const std::string& entry) {
	mEntryPointNames.push_back(entry);

	const vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = *shader,
		.pName = mEntryPointNames.back().c_str()
	};

	mShaderStages.push_back(vertShaderStageInfo);
	return *this;
}

PipelineBuilder& PipelineBuilder::addFragmentShader(Shader& shader, const std::string& entry) {
	mEntryPointNames.push_back(entry);

	const vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = *shader,
		.pName = mEntryPointNames.back().c_str()
	};

	mShaderStages.push_back(fragShaderStageInfo);
	return *this;
}

PipelineBuilder& PipelineBuilder::addComputeShader(Shader& shader, const std::string& entry) {
	mEntryPointNames.push_back(entry);

	const vk::PipelineShaderStageCreateInfo computeShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eCompute,
		.module = *shader,
		.pName = mEntryPointNames.back().c_str()
	};

	mShaderStages.push_back(computeShaderStageInfo);
	return *this;
}

PipelineBuilder& PipelineBuilder::topology(const vk::PrimitiveTopology topology) {
	mInputAssembly.topology = topology;
	mInputAssembly.primitiveRestartEnable = vk::False;

	return *this;
}

PipelineBuilder& PipelineBuilder::viewportState(const uint32_t viewportCount, const uint32_t scissorCount) {
	mViewportState.viewportCount = viewportCount;
	mViewportState.scissorCount = scissorCount;

	return *this;
}

PipelineBuilder& PipelineBuilder::rasterizer() {
	mRasterizer = {
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eNone,
		.frontFace = vk::FrontFace::eCounterClockwise,
		.depthBiasEnable = vk::False,
		.lineWidth = 1.0f
	};

	return *this;
}

PipelineBuilder& PipelineBuilder::multisampling() {
	mMultisampling = {
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False
	};

	return *this;
}

PipelineBuilder& PipelineBuilder::alphaBlending() {
	mColorBlendAttachment = {
		.blendEnable = vk::False,
		.colorWriteMask =
			vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA
	};

	mColorBlending = {
		.logicOpEnable = vk::False,
		.attachmentCount = 1,
		.pAttachments = &mColorBlendAttachment
	};

	return *this;
}

vk::raii::PipelineLayout PipelineBuilder::createPipelineLayout(const vk::DescriptorSetLayout* dscSetLayout,
                                                               const uint32_t dscSetLayoutCount,
                                                               const uint32_t pushConstantSize,
                                                               const vk::ShaderStageFlags stages) const {
	vk::PushConstantRange pushConstantRange{};
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
		.setLayoutCount = dscSetLayoutCount,
		.pSetLayouts = dscSetLayout,
	};

	if (pushConstantSize > 0) {
		pushConstantRange.stageFlags = stages;
		pushConstantRange.offset = 0;
		pushConstantRange.size = pushConstantSize;

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	}

	return {mDevice, pipelineLayoutInfo};
}

vk::raii::Pipeline PipelineBuilder::buildGraphics(vk::SurfaceFormatKHR& surfaceFormat,
                                                  const vk::raii::PipelineLayout& layout) {
	const vk::PipelineDynamicStateCreateInfo dynamicState{
		.dynamicStateCount = static_cast<uint32_t>(mDynamicStates.size()),
		.pDynamicStates = mDynamicStates.data()
	};

	const vk::StructureChain<
		vk::GraphicsPipelineCreateInfo,
		vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
		{
			.stageCount = 2,
			.pStages = mShaderStages.data(),
			.pVertexInputState = &mVertexInputInfo,
			.pInputAssemblyState = &mInputAssembly,
			.pViewportState = &mViewportState,
			.pRasterizationState = &mRasterizer,
			.pMultisampleState = &mMultisampling,
			.pColorBlendState = &mColorBlending,
			.pDynamicState = &dynamicState,
			.layout = layout,
			.renderPass = nullptr
		},
		{
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &surfaceFormat.format,
		}
	};

	return {mDevice, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>()};
}

vk::raii::Pipeline PipelineBuilder::buildCompute(const vk::raii::PipelineLayout& layout) const {
	const vk::ComputePipelineCreateInfo pipelineInfo{
		.stage = mShaderStages.front(),
		.layout = *layout
	};

	return {mDevice, nullptr, pipelineInfo};
}

GraphicsPipeline::GraphicsPipeline(PipelineBuilder& builder,
                                   Shader& shader,
                                   vk::SurfaceFormatKHR& surfaceFormat,
                                   DescriptorSetLayout& dscSetLayout,
                                   const uint32_t dscSetLayoutCount,
                                    const uint32_t pushConstantSize) {
	builder.reset();
	builder.addVertexShader(shader, "vertMain")
			.addFragmentShader(shader, "fragMain")
			.topology(vk::PrimitiveTopology::eTriangleList)
			.viewportState(1, 1)
			.dynamicStates<vk::DynamicState::eViewport, vk::DynamicState::eScissor>()
			.rasterizer()
			.multisampling()
			.alphaBlending();

	mPipelineLayout = builder.createPipelineLayout(
		&**dscSetLayout,
		dscSetLayoutCount,
		pushConstantSize,
		vk::ShaderStageFlagBits::eFragment);

	mPipeline = builder.buildGraphics(surfaceFormat, mPipelineLayout);
}

ComputePipeline::ComputePipeline(PipelineBuilder& builder,
                                 Shader& shader,
                                 DescriptorSetLayout& dscSetLayout,
                                 const uint32_t dscSetLayoutCount,
                                 const uint32_t pushConstantSize) {
	builder.reset();
	builder.addComputeShader(shader, "compMain");

	mPipelineLayout = builder.createPipelineLayout(
		&**dscSetLayout,
		dscSetLayoutCount,
		pushConstantSize,
		vk::ShaderStageFlagBits::eCompute);

	mPipeline = builder.buildCompute(mPipelineLayout);
}
