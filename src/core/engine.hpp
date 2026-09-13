#pragma once
#include <memory>

class Device;
class Window;
class Engine {
public:
    Engine();

    ~Engine();

    void run();

private:
	std::unique_ptr<Window> mWindow;
	std::unique_ptr<Device> mDevice;

    double mDeltaTime{0.0};
    double mSecondsPreviousFrame{0};
};
