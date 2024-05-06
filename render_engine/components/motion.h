#pragma once
#include "components/component.h"

#include "glm/glm.hpp"


/*
* A Motion component serves 2 purposes:
*	1. Stores motion data, including velocity and angular velocity, which other
*	components can access and modify.
*	2. When evaluated, caches the frame time, and provides an interface to access
*	and modify motion components in terms of their effect on the current frame's
*	motion step. For example, collision response components can focus on restricting
*	the object's motion in the current frame only.
* 
* Note that the Motion component DOES NOT APPLY the motion step to the object
* it's attached to. For this, an ApplyMotion component should be used, and
* components that modify motion data should be placed between the two.
* 
* Here's an example component stack that uses Motion:
*	1. Motion
*	2. KeyboardMotion
*	3. Physics/Collision
*	4. ApplyMotion
*/
class Motion : public Component {
public:

	Motion(GameObject* object);


	void setVelocity(glm::vec3 velocity);
	void setVelocity(float vx, float vy, float vz);
	void setAngularVelocity(glm::vec3 vYawPitchRoll);
	void setAngularVelocity(float vyaw, float vpitch, float vroll);

	// Returns the old value.
	glm::vec3 deltaVelocity(glm::vec3 dvelocity);
	glm::vec3 deltaVelocity(float dvx, float dvy, float dvz);
	glm::vec3 deltaAngularVelocity(glm::vec3 dvYawPitchRoll);
	glm::vec3 deltaAngularVelocity(float dvyaw, float dvpitch, float dvroll);

	glm::vec3 getVelocity();
	glm::vec3 getAngularVelocity();


	// If it cannot update (not evaluated yet), does nothing.
	void setVelocityStep(glm::vec3 velocity);
	void setVelocityStep(float vx, float vy, float vz);
	void setAngularVelocityStep(glm::vec3 vYawPitchRoll);
	void setAngularVelocityStep(float vyaw, float vpitch, float vroll);

	// Returns the old value. If it cannot update, does nothing and returns (0,0,0).
	glm::vec3 deltaVelocityStep(glm::vec3 dvelocity);
	glm::vec3 deltaVelocityStep(float dvx, float dvy, float dvz);
	glm::vec3 deltaAngularVelocityStep(glm::vec3 dvYawPitchRoll);
	glm::vec3 deltaAngularVelocityStep(float dvyaw, float dvpitch, float dvroll);

	glm::vec3 getVelocityStep();
	glm::vec3 getAngularVelocityStep();


	virtual void evaluate(float deltaTime) override;

private:

	// Nonzero means Motion has been evaluated, but ApplyMotion hasn't.
	float frameDeltaTime = 0.0f;

	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 angularVelocity = glm::vec3(0.0f);


	friend class ApplyMotion;

};



/*
* An ApplyMotion component should be placed after a Motion component, with
* components that modify motion data between them.
* 
* ApplyMotion must have a linked Motion component to pull data from. If it's
* not assigned one in the constructor, it will search for one at every evaluation
* until it finds one.
* 
* Beware of segfaults if the Motion component is removed! Use assignMotion() to
* change the assigned Motion component, or supply nullptr to disconnect it. If
* disconnected, the ApplyMotion component will search for a Motion component at
* every evaluation until it finds one.
*/
class ApplyMotion : public Component {
public:

	ApplyMotion(GameObject* object);
	ApplyMotion(GameObject* object, Motion* motion);

	void assignMotion(Motion* motion);

	virtual void evaluate(float deltaTime) override;

private:

	GameObject* thisObject = nullptr;
	Motion* thisMotion = nullptr;

};
