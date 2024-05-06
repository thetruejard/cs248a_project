#pragma once
#include "geometry/primitive.h"
#include "geometry/ellipsoid.h"
#include "graphics/mesh.h"

#include "glm/glm.hpp"


class Sphere : public Primitive {
public:

	float radius = 1.0f;
	glm::vec3 position = glm::vec3(0.0f);


	Sphere();
	Sphere(float radius);
	Sphere(float radius, glm::vec3 position);
	Sphere(float radius, float x, float y, float z);

	Ellipsoid toEllipsoid() const;


	/*
	* If mesh data already exists, it is replaced, but NOT uploaded to the GPU.
	* Returns true on success, false on failure.
	*/
	bool toMesh(Ref<Mesh> mesh, uint32_t segments, uint32_t rings) const;

};