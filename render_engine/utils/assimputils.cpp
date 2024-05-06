#include "utils/assimpUtils.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"


namespace AssimpUtils {

	glm::vec2 toVec2(const aiVector2D& v) {
		return glm::vec2(v.x, v.y);
	}
	glm::vec2 toVec2(const aiVector3D& v) {
		return glm::vec2(v.x, v.y);
	}
	glm::vec3 toVec3(const aiVector3D& v) {
		return glm::vec3(v.x, v.y, v.z);
	}
	glm::vec3 toVec3(const aiColor3D& c) {
		return glm::vec3(c.r, c.g, c.b);
	}

	glm::quat toQuat(const aiQuaternion& q) {
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	glm::mat3 toMat(const aiMatrix3x3& m) {
		return glm::transpose(glm::make_mat3(&m.a1));
	}
	glm::mat4 toMat(const aiMatrix4x4& m) {
		return glm::transpose(glm::make_mat4(&m.a1));
	}

	std::string toStr(const aiString& s) {
		const char* _str = s.C_Str();
		if (_str && *_str) {
			return std::string(_str);
		}
		return {};
	}
}