#pragma once
#include <string>

#ifndef SHADER_BINARY_DIR
#define SHADER_BINARY_DIR "shaders/"
#endif

constexpr float	ASPECT = 16.0f / 9.0f;
constexpr uint32_t WIDTH{800};
constexpr uint32_t HEIGHT{static_cast<uint32_t>(WIDTH / ASPECT)};
// Render
constexpr int32_t MAX_FRAMES_IN_FLIGHT{2};
constexpr uint32_t THREADS_PER_GROUP{10};
constexpr uint32_t TILE_SIZE = 8;
constexpr uint32_t TILES_X = (WIDTH  + TILE_SIZE - 1) / TILE_SIZE;
constexpr uint32_t TILES_Y = (HEIGHT + TILE_SIZE - 1) / TILE_SIZE;
constexpr uint32_t TILE_COUNT = TILES_X * TILES_Y;
constexpr uint32_t TILES_PER_FRAME = 64;
constexpr auto SHADER_NAME = "main.spv";
constexpr auto TEXTURE_PATH = "assets/textures/earthmap.jpg";
// Camera
constexpr float CAMERA_SPEED{2.0f};
constexpr float ZNEAR{0.1f};
constexpr float ZFAR{100000.0f};
constexpr float ZOOM{45.0f};
constexpr float MOUSE_SENSITIVITY{0.1f};
constexpr float YAW{-90.0f};
constexpr float PITCH{0.0f};

