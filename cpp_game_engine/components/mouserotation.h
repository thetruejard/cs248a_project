#pragma once
#include "components/component.h"
#include "components/motion.h"

class CppGameEngine;


class MouseRotation : public Component {
public:

	MouseRotation(GameObject* object);
	MouseRotation(GameObject* object, Motion* motion);

	virtual void evaluate(float deltaTime) override;


private:

	GameObject* thisObject = nullptr;
	Motion* thisMotion = nullptr;
	CppGameEngine* thisEngine = nullptr;

	glm::vec3 lastDeltaRot;

	void registerEventHook();

};