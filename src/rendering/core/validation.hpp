#pragma once
#include <array>

constexpr std::array validationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

constexpr std::array syncValidationFeatures = {
	vk::ValidationFeatureEnableEXT::eSynchronizationValidation
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

