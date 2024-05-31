#pragma once
#include "objects/gameobject.h"
#include "geometry/sphere.h"


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
	Type type = Type::Point;

	// Directional, Spot
	glm::vec3 direction;
	// Spot
	glm::vec2 innerOuterAngles;

	// Color (may not be clamped).
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	// Attenuation parameters: (constant, linear, quadratic).
	glm::vec3 attenuation = glm::vec3(0.0f, 0.0f, 2.0f);


	glm::vec3 getWorldSpaceDirection();

	// ASSUMES QUADRATIC ATTENUATION
	Sphere getBoundingSphere(float thresh = 0.02f);

};