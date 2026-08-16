#include "engine.h"
#include "window.h"
#include "../config/config.hpp"
#include "../rendering/device.h"

Engine::Engine()
	: mWindow(std::make_unique<Window>()),
	  mDevice(std::make_unique<Device>(*mWindow)) {
	try {
		mWindow->init("Vulkan Particles", WIDTH, HEIGHT);
		mDevice->init();
	} catch (const std::runtime_error& e) {
		throw std::runtime_error(e.what());
	}
}

Engine::~Engine() = default;

void Engine::run() {
	while (!mWindow->shouldClose()) {
		glfwPollEvents();

		const double currentTime = glfwGetTime();
		mDeltaTime = static_cast<float>(currentTime - mSecondsPreviousFrame);
		mSecondsPreviousFrame = currentTime;

		mWindow->updateFpsCounter(mDeltaTime);

		mDevice->prepareFrame();
		mDevice->submit<QueueType::Compute>(mDeltaTime);
		mDevice->submit<QueueType::Graphics>(mDeltaTime);
		mDevice->presentFrame();
	}
	mDevice->waitIdle();
}
