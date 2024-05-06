#pragma once
#include "objects/gameobject.h"


class GO_Light : public GameObject {
public:

	GO_Light(GameObjectID id, RenderEngine* engine);
	virtual ~GO_Light() override = default;
	virtual std::string getTypeName() override;

	enum class Type {
		Disabled = 0,
		Directional = 1,
		Point = 2,
		Spot = 3,
	};

	virtual void setScene(Ref<Scene> scene) override;


	// The type of light.
	Type type;

	// Directional, Spot
	glm::vec3 direction;
	// Spot
	glm::vec2 innerOuterAngles;

	// Color (may not be clamped).
	glm::vec3 color;

	// Attenuation parameters: (constant, linear, quadratic).
	glm::vec3 attenuation;


	glm::vec3 getWorldSpaceDirection();

};