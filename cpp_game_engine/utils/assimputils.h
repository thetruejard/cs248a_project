#pragma once
#include "assimp/scene.h"
#include "glm/glm.hpp"

#include <string>


namespace AssimpUtils {

	glm::vec2 toVec2(const aiVector2D& v);
	glm::vec2 toVec2(const aiVector3D& v);
	glm::vec3 toVec3(const aiVector3D& v);
	glm::vec3 toVec3(const aiColor3D& c);

	glm::quat toQuat(const aiQuaternion& q);

	glm::mat3 toMat(const aiMatrix3x3& m);
	glm::mat4 toMat(const aiMatrix4x4& m);

	std::string toStr(const aiString& s);
}