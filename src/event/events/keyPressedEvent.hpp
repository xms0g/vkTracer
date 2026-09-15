#pragma once
#include <cstdint>
#include "event.hpp"

enum class Key : uint8_t {
	W, A, S, D
};

struct KeyPressedEvent : Event {
	Key key;
	float deltaTime;

	explicit KeyPressedEvent(const Key k, const float dt)
		: key(k),
		  deltaTime(dt) {
	}
};
