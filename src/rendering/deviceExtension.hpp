#pragma once
#include <array>

constexpr std::array<const char*, 2> deviceExtensions = {
	vk::KHRSwapchainExtensionName,
	vk::KHRPortabilitySubsetExtensionName
};
