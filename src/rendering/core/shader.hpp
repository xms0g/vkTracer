#pragma once
#include <string>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Shader {
public:
	Shader(const vk::raii::Device& device, const std::string& path);

	vk::raii::ShaderModule& operator*() noexcept { return mShaderModule; }
	const vk::raii::ShaderModule& operator*() const noexcept { return mShaderModule; }

private:
	vk::raii::ShaderModule mShaderModule{nullptr};
};
