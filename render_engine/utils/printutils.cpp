#include "utils/printutils.h"
#include "objects/gameobject.h"

#include <iostream>
#include <string>


namespace Utils {
	namespace Print {

		static void objectTree_h(GameObject* obj, std::string prefix) {
			std::cout << prefix << obj->getName() << std::endl;
			for (auto& child : obj->getChildren()) {
				objectTree_h(child.get(), "   " + prefix);
			}
		}
		void objectTree(GameObject* root) {
			objectTree_h(root, "-> ");
		}

		void vec3(const glm::vec3& v) {
			std::cout << "[" << v.x << " " << v.y << " " << v.z << "]\n";
		}
		void vec4(const glm::vec4& v) {
			std::cout << "[" << v.x << " " << v.y << " " << v.z << " " << v.w << "]\n";
		}
		void mat4(const glm::mat4& mat) {
			std::cout << "[ " << mat[0][0] << " " << mat[1][0] << " " << mat[2][0] << " " << mat[3][0] << "\n";
			std::cout << "  " << mat[0][1] << " " << mat[1][1] << " " << mat[2][1] << " " << mat[3][1] << "\n";
			std::cout << "  " << mat[0][2] << " " << mat[1][2] << " " << mat[2][2] << " " << mat[3][2] << "\n";
			std::cout << "  " << mat[0][3] << " " << mat[1][3] << " " << mat[2][3] << " " << mat[3][3] << " ]\n";
		}

	}
}