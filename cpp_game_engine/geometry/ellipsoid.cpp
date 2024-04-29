#include "geometry/ellipsoid.h"

#include <iostream>

#define PI 3.141592653598f


Ellipsoid::Ellipsoid() {}
Ellipsoid::Ellipsoid(glm::vec3 radii) {
	this->radii = radii;
}
Ellipsoid::Ellipsoid(glm::vec3 radii, glm::vec3 position) {
	this->radii = radii;
	this->position = position;
}
Ellipsoid::Ellipsoid(glm::vec3 radii, float x, float y, float z) {
	this->radii = radii;
	this->position = glm::vec3(x, y, z);
}
Ellipsoid::Ellipsoid(float rx, float ry, float rz) {
	this->radii = glm::vec3(rx, ry, rz);
}
Ellipsoid::Ellipsoid(float rx, float ry, float rz, glm::vec3 position) {
	this->radii = glm::vec3(rx, ry, rz);
	this->position = position;
}
Ellipsoid::Ellipsoid(float rx, float ry, float rz, float x, float y, float z) {
	this->radii = glm::vec3(rx, ry, rz);
	this->position = glm::vec3(x, y, z);
}


bool Ellipsoid::toMesh(Ref<Mesh> mesh, uint32_t segments, uint32_t rings) const {
	if (segments < 3 || rings < 2 ||
		this->radii.x <= 0.0f || this->radii.y <= 0.0f || this->radii.z <= 0.0f) {
		return false;
	}

	VertexIndex numVerts = segments * (rings - 1) + 2;
	VertexIndex numIdxs = 6 * segments * (rings - 1);
	Vertex* verts = mesh->createVertexBuffer(numVerts);
	if (!verts) {
		return false;
	}
	VertexIndex* idxs = mesh->createIndexBuffer(numIdxs);
	if (!idxs) {
		return false;
	}

	// Vertex rings. There are (rings-1) of them.
	for (size_t r = 1; r < rings; r++) {
		size_t idxOffset = (r - 1) * segments;
		float phi = PI * (1.0f - (float)r / (float)rings);
		// Radius and y relative to unit sphere.
		float u_rad = sin(phi);
		float u_y = cos(phi);
		for (size_t s = 0; s < segments; s++) {
			float theta = 2 * PI * (float)s / (float)segments;
			verts[idxOffset + s].position = this->position + this->radii * glm::vec3(
				u_rad * cos(theta), u_y, u_rad * sin(theta)
			);
		}
	}

	// Caps.
	verts[numVerts - 2].position = this->position + glm::vec3(0.0f, -this->radii.y, 0.0f);
	verts[numVerts - 1].position = this->position + glm::vec3(0.0f, this->radii.y, 0.0f);

	// Face rings. There are (rings-2) of them.
	for (VertexIndex r = 0; r < rings - 2; r++) {
		uint32_t idxOffset = 6 * segments * r;
		// Connect first and last segments.
		idxs[idxOffset + 0] = r * segments;
		idxs[idxOffset + 1] = (r + 1) * segments;
		idxs[idxOffset + 2] = (r + 2) * segments - 1;
		idxs[idxOffset + 3] = (r + 1) * segments - 1;
		idxs[idxOffset + 4] = r * segments;
		idxs[idxOffset + 5] = (r + 2) * segments - 1;
		idxOffset += 6;
		// Connect the intermediate segments.
		for (VertexIndex s = 0; s < segments - 1; s++) {
			idxs[idxOffset + 0] = r * segments + s;
			idxs[idxOffset + 1] = r * segments + s + 1;
			idxs[idxOffset + 2] = (r + 1) * segments + s;
			idxs[idxOffset + 3] = (r + 1) * segments + s;
			idxs[idxOffset + 4] = r * segments + s + 1;
			idxs[idxOffset + 5] = (r + 1) * segments + s + 1;
			idxOffset += 6;
		}
	}

	// Caps.
	size_t capsIdxOffset = numIdxs - 6 * segments;
	// Connect first and last segments.
	// Bottom.
	idxs[capsIdxOffset + 0] = segments - 1;
	idxs[capsIdxOffset + 1] = numVerts - 2;
	idxs[capsIdxOffset + 2] = 0;
	// Top.
	idxs[capsIdxOffset + 3] = numVerts - 3;
	idxs[capsIdxOffset + 4] = numVerts - 2 - segments;
	idxs[capsIdxOffset + 5] = numVerts - 1;
	capsIdxOffset += 6;
	// Connect the intermediate segments.
	for (VertexIndex s = 0; s < segments - 1; s++) {
		// Bottom.
		idxs[capsIdxOffset + 0] = s;
		idxs[capsIdxOffset + 1] = numVerts - 2;
		idxs[capsIdxOffset + 2] = s + 1;
		// Top.
		idxs[capsIdxOffset + 3] = numVerts - 2 - segments + s;
		idxs[capsIdxOffset + 4] = numVerts - 1 - segments + s;
		idxs[capsIdxOffset + 5] = numVerts - 1;
		capsIdxOffset += 6;
	}

	// Normals.
	for (VertexIndex i = 0; i < numVerts; i++) {
		verts[i].normal = glm::normalize(
			(verts[i].position - this->position) / this->radii
		);
	}

	return true;
}