#pragma once
#include <string>

#ifndef SHADER_BINARY_DIR
#define SHADER_BINARY_DIR "shaders/"
#endif

constexpr float aspect = 16.0f / 9.0f;
constexpr uint32_t WIDTH{800};
constexpr uint32_t HEIGHT{static_cast<uint32_t>(WIDTH / aspect)};
constexpr int32_t MAX_FRAMES_IN_FLIGHT{2};
constexpr uint32_t THREADS_PER_GROUP{8};
constexpr auto SHADER_NAME = "main.spv";
