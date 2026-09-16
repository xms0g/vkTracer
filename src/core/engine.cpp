#include "engine.hpp"
#include "window.hpp"
#include "camera.hpp"
#include "input.hpp"
#include "../config/config.hpp"
#include "../rendering/device.hpp"
#include "../event/eventBus.hpp"

Engine::Engine()
	: mWindow(std::make_unique<Window>()),
	  mCamera(std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 0.0f))),
	  mDevice(std::make_unique<Device>(*mWindow, *mCamera)),
	  mEventBus(std::make_unique<EventBus>()) {
	try {
		mWindow->init("Vk Tracer", WIDTH, HEIGHT);
		mDevice->init();
	} catch (const std::runtime_error& e) {
		throw std::runtime_error(e.what());
	}
}

Engine::~Engine() = default;

void Engine::configure() const {
	mCamera->configure(*mEventBus);
}

void Engine::run() {
	while (isRunning) {
		mDeltaTime = static_cast<float>(SDL_GetTicks() - mMillisecsPreviousFrame) / 1000.0f;
		mMillisecsPreviousFrame = SDL_GetTicks();

		isRunning = Input::process(*mEventBus, &**mWindow, mDeltaTime);
		mCamera->update();

		mWindow->updateFpsCounter(mDeltaTime);

		mDevice->prepareFrame();
		mDevice->submit<QueueType::Compute>();
		mDevice->submit<QueueType::Graphics>();
		mDevice->presentFrame();
	}
	mDevice->waitIdle();
}
