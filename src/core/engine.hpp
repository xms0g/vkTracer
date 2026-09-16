#pragma once
#include <memory>

class Device;
class Window;
class EventBus;
class Camera;
class Engine {
public:
    Engine();

    ~Engine();

	void configure() const;

    void run();

private:
	std::unique_ptr<Window> mWindow;
	std::unique_ptr<Camera> mCamera;
	std::unique_ptr<Device> mDevice;
	std::unique_ptr<EventBus> mEventBus;

	bool isRunning{true};
    float mDeltaTime{0.0};
	uint32_t mMillisecsPreviousFrame{0};
    double mSecondsPreviousFrame{0};
};
