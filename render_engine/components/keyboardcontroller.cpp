#include "components/KeyboardController.h"
#include "core/renderengine.h"
#include "core/scene.h"
#include "io/inputcontext.h"
#include "objects/gameobject.h"

#include "GLFW/glfw3.h"


KeyboardController::KeyboardController(GameObject* object)
	: Component(object) {
	this->thisObject = object;
}
KeyboardController::KeyboardController(GameObject* object, Motion* motion)
	: Component(object) {
	this->thisObject = object;
	this->thisMotion = motion;
}

void KeyboardController::evaluate(float deltaTime) {

	if (!this->thisMotion) {
		this->thisMotion = this->thisObject->getComponent<Motion>();
		if (!this->thisMotion) {
			return;
		}
	}

	// TODO: Cache this whenever the scene changes, once the callback is implemented.
	Ref<Scene> scene = this->thisObject->getScene().elevate();
	InputContext* inctx = scene ? scene->getEngine()->getInputContext() : nullptr;
	if (!inctx) {
		return;
	}

	// TODO: Make these controls more flexible, i.e. as settings.

	constexpr float moveSpeed = 3.0f;
	constexpr float turnSpeed = 1.0f;

	glm::vec3 posMotion(0.0f);
	glm::vec3 rotMotion(0.0f);

	if (inctx->getKeyState(GLFW_KEY_W)) {
		posMotion += glm::vec3(0.0f, 0.0f, -1.0f);
	}
	if (inctx->getKeyState(GLFW_KEY_S)) {
		posMotion += glm::vec3(0.0f, 0.0f, 1.0f);
	}
	if (inctx->getKeyState(GLFW_KEY_A)) {
		posMotion += glm::vec3(-1.0f, 0.0f, 0.0f);
	}
	if (inctx->getKeyState(GLFW_KEY_D)) {
		posMotion += glm::vec3(1.0f, 0.0f, 0.0f);
	}
	if (inctx->getKeyState(GLFW_KEY_LEFT_SHIFT)) {
		posMotion += glm::vec3(0.0f, -1.0f, 0.0f);
	}
	if (inctx->getKeyState(GLFW_KEY_SPACE)) {
		posMotion += glm::vec3(0.0f, 1.0f, 0.0f);
	}

	if (inctx->getKeyState(GLFW_KEY_UP)) {
		rotMotion += glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (inctx->getKeyState(GLFW_KEY_DOWN)) {
		rotMotion += glm::vec3(0.0f, -1.0f, 0.0f);
	}
	if (inctx->getKeyState(GLFW_KEY_LEFT)) {
		rotMotion += glm::vec3(1.0f, 0.0f, 0.0f);
	}
	if (inctx->getKeyState(GLFW_KEY_RIGHT)) {
		rotMotion += glm::vec3(-1.0f, 0.0f, 0.0f);
	}

	glm::vec3 dir = this->thisObject->getLocalTransform().vectorApplyYaw(posMotion);
	this->thisMotion->setVelocity(dir * moveSpeed);
	this->thisMotion->setAngularVelocity(rotMotion * turnSpeed);

}