#include "window.hpp"

Window::~Window() {
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}

void Window::swapBuffer() {
	SDL_GL_SwapWindow(mWindow);
}

void Window::initImpl(const char* title, const int width, const int height, const bool fullscreen) {
	m_title = title;

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw std::runtime_error("Failed to initialize SDL_VIDEO");
	}

	uint32_t flags = SDL_WINDOW_VULKAN;

	if (fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN;
	else
		flags |= SDL_WINDOW_RESIZABLE;

	mWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

	if (!mWindow) {
		SDL_Quit();
		throw std::runtime_error(SDL_GetError());
	}
}

void Window::clearImpl(float r, float g, float b, float a) {
}

void Window::updateFpsCounter(const double dt) {
	mCurrentSeconds += dt;
	double elapsedSeconds = mCurrentSeconds - mPreviousSeconds;
	/* limit text updates to 4 per second */
	if (elapsedSeconds > 0.25) {
		mPreviousSeconds = mCurrentSeconds;
		char tmp[128];
		const double fps = static_cast<double>(mFrameCount) / elapsedSeconds;

		snprintf(tmp, 128, "%s @ fps: %.2f", m_title.c_str(), fps);

		SDL_SetWindowTitle(mWindow, tmp);
		mFrameCount = 0;
	}

	mFrameCount++;
}
