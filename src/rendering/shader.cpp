#include "shader.hpp"
#include "../io/filesystem.hpp"

Shader::Shader(const vk::raii::Device& device, const std::string& path) {
	const auto shaderCode = fs::readFile(path);

	const vk::ShaderModuleCreateInfo createInfo{
		.codeSize = shaderCode.size() * sizeof(char),
		.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data())
	};

	mShaderModule = vk::raii::ShaderModule(device, createInfo);
}
