#include "objects/go_light.h"
#include "core/scene.h"



GO_Light::GO_Light(GameObjectID id, CppGameEngine* engine)
	: GameObject(id, engine) {}

std::string GO_Light::getTypeName() {
	return "Light";
}


void GO_Light::setScene(Ref<Scene> scene) {
	if (auto s = this->thisScene.elevate()) {
		auto i = std::find(s->lights.begin(), s->lights.end(), this);
		if (i != s->lights.end()) {
			s->lights.erase(i);
		}
	}
	GameObject::setScene(scene);
	if (scene) {
		scene->lights.push_back(this);
	}
}


glm::vec3 GO_Light::getWorldSpaceDirection() {
	glm::vec4 d = this->getModelMatrix() * glm::vec4(this->direction, 0.0f);
	return glm::vec3(d.x, d.y, d.z);
}
