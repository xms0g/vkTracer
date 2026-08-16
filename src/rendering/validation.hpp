#pragma once
#include <array>

constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

