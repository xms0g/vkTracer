cmake_minimum_required(VERSION 3.23)
project(vkt CXX C)

set(CMAKE_CXX_STANDARD 23)

if (APPLE)
    add_compile_options(-gdwarf-4)
    add_definitions(-DVK_ENABLE_BETA_EXTENSIONS)
endif ()

find_package(SDL2 REQUIRED)
if (SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIR})
endif()
find_package(Vulkan REQUIRED)
find_program(SLANGC_EXECUTABLE slangc REQUIRED)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/slangHelpers.cmake)

file(GLOB_RECURSE SRC "src/*")

add_executable(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} Vulkan::Vulkan)

add_slang_shader_target(ShaderTarget
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/shaders/main.slang"
        OUTPUT_NAME "main.spv")
add_dependencies(${PROJECT_NAME} ShaderTarget)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        SHADER_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders/"
)
