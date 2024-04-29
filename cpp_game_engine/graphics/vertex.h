#pragma once
#include "glm/glm.hpp"


using VertexIndex = uint32_t;

/*
* Class representing a single vertex in a mesh.
* For now, only one texture coordinate (UV map) is supported.
* (In order to support an arbitrary number of UV maps, Mesh must be revised too.)
*/
class Vertex {
public:

	// Position.
	glm::vec3 position;
	
	// Normal: the vector pointing orthogonal to the surface, used for lighting.
	glm::vec3 normal;

	// Tangent/Bitangent: vectors pointing tangent to the surface in the direction
	// of UV coordinates, used for normal mapping (which is done in tanged space).
	glm::vec3 tangent;
	glm::vec3 bitangent;

	// UV: the texture (UV) coordinate.
	glm::vec2 uv;

};
