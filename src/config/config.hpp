#pragma once
#include <string>

#ifndef SHADER_BINARY_DIR
#define SHADER_BINARY_DIR "shaders/"
#endif

constexpr uint32_t WIDTH{800};
constexpr uint32_t HEIGHT{600};
constexpr int32_t MAX_FRAMES_IN_FLIGHT{2};
constexpr uint32_t PARTICLE_COUNT{8960};
constexpr uint32_t THREADS_PER_GROUP{256};
constexpr auto SHADER_NAME = "particle.spv";
