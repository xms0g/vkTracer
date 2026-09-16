#pragma once
#include <string>
#include <SDL.h>
#include "baseWindow.hpp"

class Window : public BaseWindow<SDL_Window> {
public:
	Window() = default;

	~Window() override;

	void updateFpsCounter(double dt);

	void swapBuffer() override;

protected:
	void initImpl(const char* title, int width, int height, bool fullscreen) override;

	void clearImpl(float r, float g, float b, float a) override;

	std::string m_title;

	double mPreviousSeconds{0.0};
	double mCurrentSeconds{0.0};
	int mFrameCount{0};
};
