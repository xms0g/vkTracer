#include "window.h"

Window::~Window() {
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

bool Window::shouldClose() const {
	return glfwWindowShouldClose(mWindow);
}

bool Window::windowResized() const {
	return mWindowResized;
}

void Window::windowResized(const bool resized) {
	mWindowResized = resized;
}

void Window::swapBuffer() {
	glfwSwapBuffers(mWindow);
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	const auto myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	myWindow->windowResized(true);

	glfwGetFramebufferSize(window, &width, &height);

	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
}

void Window::initImpl(const char* title, const int width, const int height, bool fullscreen) {
	m_title = title;

	if (glfwInit() != GLFW_TRUE) {
		throw std::runtime_error("Failed to initialize Window");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
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

		glfwSetWindowTitle(mWindow, tmp);
		mFrameCount = 0;
	}

	mFrameCount++;
}
