#include "geometry/rectangle.h"


Rectangle::Rectangle() {}
Rectangle::Rectangle(glm::vec2 bottomLeft, glm::vec2 widthHeight) {
	this->bottomLeft = bottomLeft;
	this->widthHeight = widthHeight;
}
Rectangle::Rectangle(float bottom, float left, float width, float height) {
	this->bottomLeft = glm::vec2(bottom, left);
	this->widthHeight = glm::vec2(width, height);
}


void Rectangle::correctNegativeSideLengths() {
	if (this->widthHeight.x < 0.0f) {
		this->bottomLeft.x += this->widthHeight.x;
		this->widthHeight.x *= -1.0f;
	}
	if (this->widthHeight.y < 0.0f) {
		this->bottomLeft.y += this->widthHeight.y;
		this->widthHeight.y *= -1.0f;
	}
}


bool Rectangle::toMesh(Ref<Mesh> mesh) const {
	if (!mesh) {
		return false;
	}
	Vertex* verts = mesh->createVertexBuffer(4);
	if (!verts) {
		return false;
	}
	verts[0].position = glm::vec3(
		this->bottomLeft, 0.0f);
	verts[0].uv = glm::vec2(0.0f, 0.0f);
	verts[1].position = glm::vec3(
		this->bottomLeft.x + this->widthHeight.x, this->bottomLeft.y, 0.0f);
	verts[1].uv = glm::vec2(1.0f, 0.0f);
	verts[2].position = glm::vec3(
		this->bottomLeft.x, this->bottomLeft.y + this->widthHeight.y, 0.0f);
	verts[2].uv = glm::vec2(0.0f, 1.0f);
	verts[3].position = glm::vec3(
		this->bottomLeft + this->widthHeight, 0.0f);
	verts[3].uv = glm::vec2(1.0f, 1.0f);
	VertexIndex* idxs = mesh->createIndexBuffer(6);
	if (!idxs) {
		return false;
	}
	idxs[0] = 0;
	idxs[1] = 1;
	idxs[2] = 2;
	idxs[3] = 1;
	idxs[4] = 3;
	idxs[5] = 2;
	return true;
}
