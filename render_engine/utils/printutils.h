#pragma once
#include "glm/glm.hpp"

#include <memory>


class GameObject;


namespace Utils {
	namespace Print {

		void objectTree(GameObject* root);

		void mat4(const glm::mat4& mat);

	}
}