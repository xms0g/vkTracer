#pragma once

class EventBus;
class Window;

namespace Input {
bool process(EventBus& eventBus, Window& window, float dt);
}
