#pragma once
#include "glm/glm.hpp"


class Transform {
public:

	/*
	* Set.
	*/

	void setPosition(glm::vec3 position);
	void setPosition(float x, float y, float z);

	void setRotation(glm::vec3 euler);
	void setRotation(float yaw, float pitch, float roll);

	void setScale(glm::vec3 scale);
	void setScale(float x, float y, float z);

	/*
	* Delta.
	* Offset current value and return the OLD value.
	* "Offset" means add for pos/rot and multiply for scale.
	*/

	glm::vec3 deltaPosition(glm::vec3 dposition);
	glm::vec3 deltaPosition(float dx, float dy, float dz);

	glm::vec3 deltaRotation(glm::vec3 deuler);
	glm::vec3 deltaRotation(float dyaw, float dpitch, float droll);

	glm::vec3 deltaScale(glm::vec3 dscale);
	glm::vec3 deltaScale(float dx, float dy, float dz);

	/*
	* Get.
	*/

	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	glm::vec3 getScale() const;

	/*
	* Other.
	*/

	void fromMatrix(const glm::mat4& mat);
	void clear();

	glm::mat4 getMatrix();
	

	glm::vec3 transformVector(glm::vec3 vector);
	// Only rotates the vector, without translating or scaling.
	glm::vec3 rotateVector(glm::vec3 vector);
	// Rotate while considering yaw-pitch for FBLR, but NOT for up-down.
	// Use this to get FPS movement offsets like Blender's first-person camera.
	glm::vec3 vectorApplyYawPitch(glm::vec3 vector);
	// Rotates while considering yaw rotation only.
	// Use this to get GPS movement offsets like Minecraft's first-person camera.
	glm::vec3 vectorApplyYaw(glm::vec3 vector);


private:

	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);		// Euler (yaw, pitch, roll)
	glm::vec3 scale = glm::vec3(1.0f);

	glm::mat4 cachedMatrix;
	bool dirty = true;

};