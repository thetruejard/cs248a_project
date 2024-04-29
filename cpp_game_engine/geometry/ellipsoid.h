#pragma once
#include "geometry/primitive.h"
#include "graphics/mesh.h"

#include "glm/glm.hpp"


class Ellipsoid : public Primitive {
public:

	glm::vec3 radii = glm::vec3(1.0f);
	glm::vec3 position = glm::vec3(0.0f);


	Ellipsoid();
	Ellipsoid(glm::vec3 radii);
	Ellipsoid(glm::vec3 radii, glm::vec3 position);
	Ellipsoid(glm::vec3 radii, float x, float y, float z);
	Ellipsoid(float rx, float ry, float rz);
	Ellipsoid(float rx, float ry, float rz, glm::vec3 position);
	Ellipsoid(float rx, float ry, float rz, float x, float y, float z);



	/*
	* If mesh data already exists, it is replaced, but NOT uploaded to the GPU.
	* Returns true on success, false on failure.
	* TODO: For now, only supports positions and normals. Later, consider UVs and tangents.
	*/
	bool toMesh(Ref<Mesh> mesh, uint32_t segments, uint32_t rings) const;

};