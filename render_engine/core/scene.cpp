#include "core/renderengine.h"
#include "core/scene.h"


Scene::Scene(SceneID id, RenderEngine* engine) : Datablock(id), thisEngine(engine) {
	if (this->thisEngine) {
		this->root = this->thisEngine->createObject<GameObject>();
	}
}


bool Scene::addObject(Ref<GameObject> object) {
	if (object && !object->getParent()) {
		object->setParent(this->root, false);
		object->setScene(this->getRef());
		return true;
	}
	return false;
}


Ref<GameObject> Scene::getRoot() {
	return this->root;
}


void Scene::setActiveCamera(Ref<GO_Camera> camera) {
	this->activeCamera = camera.weak();
}

Ref<GO_Camera> Scene::getActiveCamera() {
	return this->activeCamera.elevate();
}


static void evalComps(GameObject* object, float deltaTime) {
	for (Component* component : object->getComponents()) {
		component->evaluate(deltaTime);
	}
	for (const Ref<GameObject>& child : object->getChildren()) {
		evalComps(child.get(), deltaTime);
	}
}

void Scene::evaluateComponents(float deltaTime) {
	evalComps(this->root.get(), deltaTime);
}



RenderEngine* Scene::getEngine() {
	return this->thisEngine;
}
