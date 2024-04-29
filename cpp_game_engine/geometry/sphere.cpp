#include "geometry/sphere.h"


Sphere::Sphere() {}
Sphere::Sphere(float radius) {
	this->radius = radius;
}
Sphere::Sphere(float radius, glm::vec3 position) {
	this->radius = radius;
	this->position = position;
}
Sphere::Sphere(float radius, float x, float y, float z) {
	this->radius = radius;
	this->position = glm::vec3(x, y, z);
}

Ellipsoid Sphere::toEllipsoid() const {
	return Ellipsoid(glm::vec3(this->radius), this->position);
}


bool Sphere::toMesh(Ref<Mesh> mesh, uint32_t segments, uint32_t rings) const {
	return this->toEllipsoid().toMesh(mesh, segments, rings);
}