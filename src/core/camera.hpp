#pragma once
#include <glm/glm.hpp>

struct MouseMovementEvent;
struct KeyPressedEvent;
class EventBus;

class Camera {
public:
	explicit Camera(const glm::vec3& center = glm::vec3{0.0f, 0.0f, 0.0f},
	                const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

	[[nodiscard]]
	glm::vec3 center() const;

	[[nodiscard]]
	glm::vec3 front() const;

	[[nodiscard]]
	glm::vec3 right() const;

	[[nodiscard]]
	glm::vec3 up() const;

	void configure(EventBus& eventBus);

	void update();

	void processKeyboard(const KeyPressedEvent& event);

	void processMouseMovement(const MouseMovementEvent& event);

private:
	// camera Attributes
	glm::vec3 mCenter{};
	glm::vec3 mFront{};
	glm::vec3 mUp{};
	glm::vec3 mRight{};
	glm::vec3 mWorldUp{};
	// euler Angles
	float mYaw{};
	float mPitch{};
	// camera options
	float mZFar{};
	float mZNear{};
	float mZoom{};
	float mMovementSpeed{};
	float mMouseSensitivity{};
};
