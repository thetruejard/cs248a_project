#include "objects/go_camera.h"
#include "core/renderengine.h"

#include "glm/ext/matrix_clip_space.hpp"


GO_Camera::GO_Camera(GameObjectID id, RenderEngine* engine)
	: GameObject(id, engine) {
	engine->getDepsgraph()->hookEvent(
		[this](const StandardEvents::ResizeFramebuffer* e) {
			this->setAspect(float(e->width) / float(e->height));
		}
	);
}

std::string GO_Camera::getTypeName() {
	return "Camera";
}

void GO_Camera::setModelMatrixDirty() {
	this->viewMatrixDirty = true;
	GameObject::setModelMatrixDirty();
}


const glm::mat4& GO_Camera::getViewMatrix() {
	if (this->viewMatrixDirty) {
		this->viewMatrix = glm::inverse(this->getModelMatrix());
		this->viewMatrixDirty = false;
	}
	return this->viewMatrix;
}

const glm::mat4& GO_Camera::getProjectionMatrix() {
	return this->projectionMatrix;
}

void GO_Camera::setOrthographic(float yradius, float aspect, float near, float far) {
	this->projectionType = ProjectionType::Ortho_yradius;
	float xradius = yradius * aspect;
	this->projectionMatrix = glm::ortho(
		-xradius, xradius,
		-yradius, yradius,
		near, far
	);
	this->projectionParams.ortho_yradius.yradius = yradius;
	this->projectionParams.ortho_yradius.near = near;
	this->projectionParams.ortho_yradius.far = far;
}

void GO_Camera::setOrthographic(
	float left, float right, float top, float bottom, float near, float far
) {
	this->projectionType = ProjectionType::Ortho_planes;
	this->projectionMatrix - glm::ortho(left, right, top, bottom, near, far);
}

void GO_Camera::setPerspective(float fovy, float aspect, float near, float far) {
	this->projectionType = ProjectionType::Perspective;
	this->projectionMatrix = glm::perspective(fovy, aspect, near, far);
	this->projectionParams.perspective.fovy = fovy;
	this->projectionParams.perspective.near = near;
	this->projectionParams.perspective.far = far;
	this->projectionParams.perspective.aspect = aspect;
}

void GO_Camera::setCustomProjection(const glm::mat4& proj) {
	this->projectionType = ProjectionType::Custom;
	this->projectionMatrix = proj;
}

void GO_Camera::setAspect(float aspect) {
	switch (this->projectionType) {
	case ProjectionType::Ortho_yradius:
		this->setOrthographic(
			this->projectionParams.ortho_yradius.yradius,
			aspect,
			this->projectionParams.ortho_yradius.near,
			this->projectionParams.ortho_yradius.far
		);
		break;
	case ProjectionType::Perspective:
		this->setPerspective(
			this->projectionParams.perspective.fovy,
			aspect,
			this->projectionParams.perspective.near,
			this->projectionParams.perspective.far
		);
		break;
	default:
		break;
	}
}
