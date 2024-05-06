#include "components/motion.h"
#include "objects/gameobject.h"

#include <iostream>


Motion::Motion(GameObject* object) : Component(object) {}


void Motion::setVelocity(glm::vec3 velocity) {
	this->velocity = velocity;
}
void Motion::setVelocity(float vx, float vy, float vz) {
	this->velocity = glm::vec3(vx, vy, vz);
}
void Motion::setAngularVelocity(glm::vec3 vYawPitchRoll) {
	this->angularVelocity = vYawPitchRoll;
}
void Motion::setAngularVelocity(float vyaw, float vpitch, float vroll) {
	this->angularVelocity = glm::vec3(vyaw, vpitch, vroll);
}

// Returns the old value.
glm::vec3 Motion::deltaVelocity(glm::vec3 dvelocity) {
	glm::vec3 old = this->velocity;
	this->velocity += dvelocity;
	return old;
}
glm::vec3 Motion::deltaVelocity(float dvx, float dvy, float dvz) {
	glm::vec3 old = this->velocity;
	this->velocity += glm::vec3(dvx, dvy, dvz);
	return old;
}
glm::vec3 Motion::deltaAngularVelocity(glm::vec3 dvYawPitchRoll) {
	glm::vec3 old = this->angularVelocity;
	this->angularVelocity += dvYawPitchRoll;
	return old;
}
glm::vec3 Motion::deltaAngularVelocity(float dvyaw, float dvpitch, float dvroll) {
	glm::vec3 old = this->angularVelocity;
	this->angularVelocity += glm::vec3(dvyaw, dvpitch, dvroll);
	return old;
}

glm::vec3 Motion::getVelocity() {
	return this->velocity;
}
glm::vec3 Motion::getAngularVelocity() {
	return this->angularVelocity;
}


void Motion::setVelocityStep(glm::vec3 velocity) {
	if (this->frameDeltaTime > 0) {
		this->velocity = velocity / this->frameDeltaTime;
	}
}
void Motion::setVelocityStep(float vx, float vy, float vz) {
	if (this->frameDeltaTime > 0) {
		this->velocity = glm::vec3(vx, vy, vz) / this->frameDeltaTime;
	}
}
void Motion::setAngularVelocityStep(glm::vec3 vYawPitchRoll) {
	if (this->frameDeltaTime > 0) {
		this->angularVelocity = vYawPitchRoll / this->frameDeltaTime;
	}
}
void Motion::setAngularVelocityStep(float vyaw, float vpitch, float vroll) {
	if (this->frameDeltaTime > 0) {
		this->angularVelocity = glm::vec3(vyaw, vpitch, vroll) / this->frameDeltaTime;
	}
}

// Returns the old value.
glm::vec3 Motion::deltaVelocityStep(glm::vec3 dvelocity) {
	if (this->frameDeltaTime > 0) {
		glm::vec3 old = this->velocity;
		this->velocity += dvelocity / this->frameDeltaTime;
		return old;
	}
	return glm::vec3(0.0f, 0.0f, 0.0f);
}
glm::vec3 Motion::deltaVelocityStep(float dvx, float dvy, float dvz) {
	return this->deltaAngularVelocityStep(glm::vec3(dvx, dvy, dvz));
}
glm::vec3 Motion::deltaAngularVelocityStep(glm::vec3 dvYawPitchRoll) {
	if (this->frameDeltaTime > 0) {
		glm::vec3 old = this->angularVelocity;
		this->angularVelocity += dvYawPitchRoll / this->frameDeltaTime;
		return old;
	}
	return glm::vec3(0.0f, 0.0f, 0.0f);
}
glm::vec3 Motion::deltaAngularVelocityStep(float dvyaw, float dvpitch, float dvroll) {
	return this->deltaAngularVelocityStep(glm::vec3(dvyaw, dvpitch, dvroll));
}

glm::vec3 Motion::getVelocityStep() {
	return this->velocity * this->frameDeltaTime;
}
glm::vec3 Motion::getAngularVelocityStep() {
	return this->angularVelocity * this->frameDeltaTime;
}


void Motion::evaluate(float deltaTime) {
	this->frameDeltaTime = deltaTime;
}



ApplyMotion::ApplyMotion(GameObject* object) : Component(object) {
	this->thisObject = object;
	this->thisMotion = this->thisObject->getComponent<Motion>();
}
ApplyMotion::ApplyMotion(GameObject* object, Motion* motion) : Component(object) {
	this->thisObject = object;
	this->thisMotion = motion;
}

void ApplyMotion::assignMotion(Motion* motion) {
	this->thisMotion = motion;
}

void ApplyMotion::evaluate(float deltaTime) {
	if (!this->thisMotion) {
		this->thisMotion = this->thisObject->getComponent<Motion>();
		if (!this->thisMotion) {
			return;
		}
	}
	this->thisObject->deltaPosition(this->thisMotion->getVelocityStep());
	this->thisObject->deltaRotation(this->thisMotion->getAngularVelocityStep());
}