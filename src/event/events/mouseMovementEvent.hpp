#pragma once
#include "event.hpp"

struct MouseMovementEvent : Event {
	float x;
	float y;

	explicit MouseMovementEvent(const float x_, const float y_)
		: x(x_),
		  y(y_) {
	}
};
