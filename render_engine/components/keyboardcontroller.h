#pragma once
#include "components/component.h"
#include "components/motion.h"


class KeyboardController : public Component {
public:

	KeyboardController(GameObject* object);
	KeyboardController(GameObject* object, Motion* motion);

	virtual void evaluate(float deltaTime) override;


private:

	GameObject* thisObject = nullptr;
	Motion* thisMotion = nullptr;

};
