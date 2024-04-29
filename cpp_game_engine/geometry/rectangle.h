#pragma once
#include "geometry/primitive.h"
#include "graphics/mesh.h"

#include "glm/glm.hpp"


/*
* 2D rectangle defined by a bottom-left corner and width/height.
* A rectangle with negative side lengths is ill-formed and may not work with
* all algorithms. See correctNegativeSideLengths().
*/
class Rectangle : public Primitive {
public:

	glm::vec2 bottomLeft = glm::vec2(0.0f, 0.0f);
	glm::vec2 widthHeight = glm::vec2(1.0f, 1.0f);


	Rectangle();
	Rectangle(glm::vec2 bottomLeft, glm::vec2 widthHeight);
	Rectangle(float bottom, float left, float width, float height);

	/*
	* Corrects negative side lengths by moving the bottomLeft position such that
	* the same area is enclosed, but all side lengths are positive.
	*/
	void correctNegativeSideLengths();


	/*
	* Builds a mesh on the plane z=0.
	* If mesh data already exists, it is replaced, but NOT uploaded to the GPU.
	* Returns true on success, false on failure.
	*/
	bool toMesh(Ref<Mesh> mesh) const;

};