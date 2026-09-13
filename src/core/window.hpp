#pragma once
#include <string>
#define GLFW_INCLUDE_VULKAN  
#include <GLFW/glfw3.h>
#include "baseWindow.hpp"

class Window : public BaseWindow<GLFWwindow> {
public:
	Window() = default;

	~Window() override;

	void updateFpsCounter(double dt);

	[[nodiscard]]
	bool shouldClose() const;

	[[nodiscard]]
	bool windowResized() const;

	void windowResized(bool resized);

	void swapBuffer() override;

protected:
	void initImpl(const char* title, int width, int height, bool fullscreen) override;

	void clearImpl(float r, float g, float b, float a) override;

	std::string m_title;

	double mPreviousSeconds{0.0};
	double mCurrentSeconds{0.0};
	int mFrameCount{0};

private:
	static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

	bool mWindowResized{false};
};
