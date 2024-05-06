#include "objects/go_mesh.h"
#include "core/renderengine.h"


GO_Mesh::GO_Mesh(GameObjectID id, RenderEngine* engine) :
	GameObject(id, engine) {}

std::string GO_Mesh::getTypeName() {
	return "Mesh";
}

void GO_Mesh::draw() {
	if (this->mesh) {
		this->mesh->draw();
	}
}

void GO_Mesh::assignMesh(const Ref<Mesh>& mesh) {
	this->mesh = mesh;
}