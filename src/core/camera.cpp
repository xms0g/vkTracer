#include "camera.hpp"
#include <algorithm>
#include "../config/config.hpp"
#include "../event/eventBus.hpp"
#include "../event/events/keyPressedEvent.hpp"
#include "../event/events/mouseMovementEvent.hpp"

Camera::Camera(const glm::vec3& center, const glm::vec3& up)
	: mCenter(center),
	  mFront(0.0f, 0.0f, -1.0f),
	  mWorldUp(up),
	  mYaw(YAW),
	  mPitch(PITCH),
	  mZFar(ZFAR),
	  mZNear(ZNEAR),
	  mZoom(ZOOM),
	  mMovementSpeed(CAMERA_SPEED),
	  mMouseSensitivity(MOUSE_SENSITIVITY) {
}

glm::vec3 Camera::center() const {
	return mCenter;
}

glm::vec3 Camera::front() const {
	return mFront;
}

glm::vec3 Camera::right() const {
	return mRight;
}

glm::vec3 Camera::up() const {
	return mUp;
}

void Camera::configure(EventBus& eventBus) {
	update();

	eventBus.subscribeToEvent<Camera, KeyPressedEvent>(this, &Camera::processKeyboard);
	eventBus.subscribeToEvent<Camera, MouseMovementEvent>(this, &Camera::processMouseMovement);
}

void Camera::update() {
	// calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	front.y = sin(glm::radians(mPitch));
	front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	mFront = glm::normalize(front);
	// also re-calculate the Right and Up vector
	// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	mRight = glm::normalize(glm::cross(mFront, mWorldUp));
	mUp = glm::normalize(glm::cross(mRight, mFront));
}

void Camera::processKeyboard(const KeyPressedEvent& event) {
	const float velocity = mMovementSpeed * event.deltaTime;

	if (event.key == Key::W)
		mCenter += mFront * velocity;
	if (event.key == Key::S)
		mCenter -= mFront * velocity;
	if (event.key == Key::A)
		mCenter -= mRight * velocity;
	if (event.key == Key::D)
		mCenter += mRight * velocity;
}

void Camera::processMouseMovement(const MouseMovementEvent& event) {
	float xoffset = event.x;
	float yoffset = event.y;

	xoffset *= mMouseSensitivity;
	yoffset *= mMouseSensitivity;

	mYaw += xoffset;
	mPitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	mPitch = std::clamp(mPitch, -89.0f, 89.0f);
}
