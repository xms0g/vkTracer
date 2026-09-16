#include "input.hpp"
#include <SDL.h>
#include "../event/eventBus.hpp"
#include "../event/events/keyPressedEvent.hpp"
#include "../event/events/mouseMovementEvent.hpp"

bool Input::process(EventBus& eventBus, SDL_Window* window, const float dt) {
	SDL_Event event;
	static bool freeLook{false};

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				return false;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
					return false;
				}
			case SDL_MOUSEMOTION:
				if (freeLook) {
					eventBus.emitEvent<MouseMovementEvent>(
						static_cast<float>(event.motion.xrel),
						static_cast<float>(-event.motion.yrel));
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_RIGHT) {
					freeLook = !freeLook;
					SDL_SetRelativeMouseMode(freeLook ? SDL_TRUE : SDL_FALSE);
				}
				break;
		}
	}

	const auto* keyState = SDL_GetKeyboardState(nullptr);

	if (keyState[SDL_SCANCODE_ESCAPE]) return false;
	if (keyState[SDL_SCANCODE_W]) eventBus.emitEvent<KeyPressedEvent>(Key::W, dt);
	if (keyState[SDL_SCANCODE_S]) eventBus.emitEvent<KeyPressedEvent>(Key::S, dt);
	if (keyState[SDL_SCANCODE_A]) eventBus.emitEvent<KeyPressedEvent>(Key::A, dt);
	if (keyState[SDL_SCANCODE_D]) eventBus.emitEvent<KeyPressedEvent>(Key::D, dt);

	return true;
}
