#include "components/mouserotation.h"
#include "core/renderengine.h"
#include "core/scene.h"
#include "depsgraph/depsgraph.h"
#include "io/inputcontext.h"
#include "objects/GameObject.h"

#include "GLFW/glfw3.h"

#include <iostream>

#define PI 3.141592653598f


MouseRotation::MouseRotation(GameObject* object)
	: Component(object) {
	this->thisObject = object;
	if (auto s = this->thisObject->getScene().elevate()) {
		this->thisEngine = s->getEngine();
	}
	this->registerEventHook();
}
MouseRotation::MouseRotation(GameObject* object, Motion* motion)
	: Component(object) {
	this->thisObject = object;
	if (auto s = this->thisObject->getScene().elevate()) {
		this->thisEngine = s->getEngine();
	}
	this->thisMotion = motion;
	this->registerEventHook();
}

void MouseRotation::evaluate(float deltaTime) {

	if (!this->thisMotion) {
		this->thisMotion = this->thisObject->getComponent<Motion>();
		if (!this->thisMotion) {
			return;
		}
	}

	if (auto s = this->thisObject->getScene().elevate(); s && !this->thisEngine) {
		this->thisEngine = s->getEngine();
		this->registerEventHook();
	}

	this->thisMotion->deltaAngularVelocity(this->lastDeltaRot);
	this->lastDeltaRot = glm::vec3(0.0f);

}


void MouseRotation::registerEventHook() {

	if (!this->thisEngine) {
		return;
	}

	this->thisEngine->getDepsgraph()->hookEvent(
		[this](const StandardEvents::CursorMove* e) {

			if (!this->thisMotion) {
				return;
			}

			constexpr float turnSensitivity = 0.70f;
			if (glfwGetInputMode(
				this->thisEngine->getGraphics()->getWindow(),
				GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
				return;
			}
			this->lastDeltaRot = glm::vec3(
				-e->delta * turnSensitivity, 0.0f
			);
		}
	);

}
