#include "input.hpp"
#include <GLFW/glfw3.h>
#include "window.hpp"
#include "../event/eventBus.hpp"
#include "../event/events/mouseMovementEvent.hpp"
#include "../event/events/keyPressedEvent.hpp"

bool Input::process(EventBus& eventBus, Window& window, float dt) {
	static bool freeLook{false};

	glfwPollEvents();

	if (glfwWindowShouldClose(&*window))
		return false;

	if (glfwGetKey(&*window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		return false;

	if (glfwGetKey(&*window, GLFW_KEY_W) == GLFW_PRESS)
		eventBus.emitEvent<KeyPressedEvent>(Key::W, dt);

	if (glfwGetKey(&*window, GLFW_KEY_S) == GLFW_PRESS)
		eventBus.emitEvent<KeyPressedEvent>(Key::S, dt);

	if (glfwGetKey(&*window, GLFW_KEY_A) == GLFW_PRESS)
		eventBus.emitEvent<KeyPressedEvent>(Key::A, dt);

	if (glfwGetKey(&*window, GLFW_KEY_D) == GLFW_PRESS)
		eventBus.emitEvent<KeyPressedEvent>(Key::D, dt);

	return true;
}
