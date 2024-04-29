#include "core/transform.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"


void Transform::setPosition(glm::vec3 position) {
	this->dirty = true;
	this->position = position;
}
void Transform::setPosition(float x, float y, float z) {
	this->dirty = true;
	this->position = glm::vec3(x, y, z);
}

void Transform::setRotation(glm::vec3 euler) {
	this->dirty = true;
	this->rotation = euler;
}
void Transform::setRotation(float yaw, float pitch, float roll) {
	this->dirty = true;
	this->rotation = glm::vec3(yaw, pitch, roll);
}

void Transform::setScale(glm::vec3 scale) {
	this->dirty = true;
	this->scale = scale;
}
void Transform::setScale(float x, float y, float z) {
	this->dirty = true;
	this->scale = glm::vec3(x, y, z);
}


glm::vec3 Transform::deltaPosition(glm::vec3 dposition) {
	this->dirty = true;
	glm::vec3 old = this->position;
	this->position += dposition;
	return old;
}
glm::vec3 Transform::deltaPosition(float dx, float dy, float dz) {
	this->dirty = true;
	glm::vec3 old = this->position;
	this->position += glm::vec3(dx, dy, dz);
	return old;
}

glm::vec3 Transform::deltaRotation(glm::vec3 deuler) {
	this->dirty = true;
	glm::vec3 old = this->rotation;
	this->rotation += deuler;
	return old;
}
glm::vec3 Transform::deltaRotation(float dyaw, float dpitch, float droll) {
	this->dirty = true;
	glm::vec3 old = this->rotation;
	this->rotation += glm::vec3(dyaw, dpitch, droll);
	return old;
}

glm::vec3 Transform::deltaScale(glm::vec3 dscale) {
	this->dirty = true;
	glm::vec3 old = this->scale;
	this->scale *= dscale;
	return old;
}
glm::vec3 Transform::deltaScale(float dx, float dy, float dz) {
	this->dirty = true;
	glm::vec3 old = this->scale;
	this->scale *= glm::vec3(dx, dy, dz);
	return old;
}


glm::vec3 Transform::getPosition() const {
	return this->position;
}
glm::vec3 Transform::getRotation() const {
	return  this->rotation;
}
glm::vec3 Transform::getScale() const {
	return this->scale;
}


void Transform::fromMatrix(const glm::mat4& mat) {
	// @source (modified): https://stackoverflow.com/a/68323550
	this->dirty = false;
	this->cachedMatrix = mat;
	this->position = mat[3];
	for (int i = 0; i < 3; i++) {
		this->scale[i] = glm::length(glm::vec3(mat[i]));
	}
	const glm::mat3 rotMtx(
		glm::vec3(mat[0]) / scale[0],
		glm::vec3(mat[1]) / scale[1],
		glm::vec3(mat[2]) / scale[2]
	);
	glm::extractEulerAngleYXZ(glm::mat4(rotMtx),
		this->rotation.x, this->rotation.y, this->rotation.z);
}
void Transform::clear() {
	this->dirty = true;
	this->position = glm::vec3(0.0f);
	this->rotation = glm::vec3(0.0f);
	this->scale = glm::vec3(1.0f);
}

glm::mat4 Transform::getMatrix() {
	if (dirty) {
		glm::mat4 eye(1.0f);
		this->cachedMatrix = glm::translate(eye, this->position) *
			glm::yawPitchRoll(this->rotation.x, this->rotation.y, this->rotation.z) *
			glm::scale(eye, this->scale);
		dirty = false;
	}
	return cachedMatrix;
}


glm::vec3 Transform::transformVector(glm::vec3 vector) {
	glm::vec4 t = this->getMatrix() * glm::vec4(vector, 1.0f);
	return glm::vec3(t.x, t.y, t.z);
}
glm::vec3 Transform::rotateVector(glm::vec3 vector) {
	return glm::mat3(glm::yawPitchRoll(
		this->rotation.x, this->rotation.y, this->rotation.z
	)) * vector;
}
glm::vec3 Transform::vectorApplyYawPitch(glm::vec3 vector) {
	glm::vec3 FBLR = glm::mat3(glm::yawPitchRoll(
		this->rotation.x, this->rotation.y, this->rotation.z
	)) * glm::vec3(vector.x, 0.0f, vector.z);
	return glm::vec3(FBLR.x, FBLR.y + vector.y, FBLR.z);
}
glm::vec3 Transform::vectorApplyYaw(glm::vec3 vector) {
	glm::mat2 rotMat = glm::mat2(glm::rotate(glm::mat3(1.0f), -this->rotation.x));
	glm::vec2 rotatedXZ = rotMat * glm::vec2(vector.x, vector.z);
	return glm::vec3(rotatedXZ.x, vector.y, rotatedXZ.y);
}
