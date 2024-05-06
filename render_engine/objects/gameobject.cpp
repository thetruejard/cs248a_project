#include "objects/gameobject.h"


GameObject::GameObject(GameObjectID id, RenderEngine* engine) : Datablock(id) {}
GameObject::~GameObject() {
	this->clearComponents();
}

std::string GameObject::getTypeName() {
	return "GameObject";
}

std::string GameObject::getName() {
	return this->name;
}
void GameObject::setName(std::string name) {
	this->name = name;
}

void GameObject::setScene(Ref<Scene> scene) {
	this->thisScene = scene.weak();
	for (Ref<GameObject>& child : this->children) {
		child->setScene(scene);
	}
}

WeakRef<Scene> GameObject::getScene() {
	return this->thisScene;
}


void GameObject::setPosition(glm::vec3 position) {
	this->setModelMatrixDirty();
	this->transform.setPosition(position);
}
void GameObject::setPosition(float x, float y, float z) {
	this->setModelMatrixDirty();
	this->transform.setPosition(x, y, z);
}
void GameObject::setRotation(glm::vec3 euler) {
	this->setModelMatrixDirty();
	this->transform.setRotation(euler);
}
void GameObject::setRotation(float yaw, float pitch, float roll) {
	this->setModelMatrixDirty();
	this->transform.setRotation(yaw, pitch, roll);
}
void GameObject::setScale(glm::vec3 scale) {
	this->setModelMatrixDirty();
	this->transform.setScale(scale);
}
void GameObject::setScale(float x, float y, float z) {
	this->setModelMatrixDirty();
	this->transform.setScale(x, y, z);
}

glm::vec3 GameObject::deltaPosition(glm::vec3 dposition) {
	this->setModelMatrixDirty();
	return this->transform.deltaPosition(dposition);
}
glm::vec3 GameObject::deltaPosition(float dx, float dy, float dz) {
	this->setModelMatrixDirty();
	return this->transform.deltaPosition(dx, dy, dz);
}
glm::vec3 GameObject::deltaPositionWithRot(glm::vec3 dposition) {
	this->setModelMatrixDirty();
	glm::vec3 v = this->transform.rotateVector(dposition);
	return this->transform.deltaPosition(v);
}
glm::vec3 GameObject::deltaPositionWithYawPitch(glm::vec3 dposition) {
	this->setModelMatrixDirty();
	glm::vec3 v = this->transform.vectorApplyYawPitch(dposition);
	return this->transform.deltaPosition(v);
}
glm::vec3 GameObject::deltaPositionWithYaw(glm::vec3 dposition) {
	this->setModelMatrixDirty();
	glm::vec3 v = this->transform.vectorApplyYaw(dposition);
	return this->transform.deltaPosition(v);
}
glm::vec3 GameObject::deltaRotation(glm::vec3 deuler) {
	this->setModelMatrixDirty();
	return this->transform.deltaRotation(deuler);
}
glm::vec3 GameObject::deltaRotation(float dyaw, float dpitch, float droll) {
	this->setModelMatrixDirty();
	return this->transform.deltaRotation(dyaw, dpitch, droll);
}
glm::vec3 GameObject::deltaScale(glm::vec3 dscale) {
	this->setModelMatrixDirty();
	return this->transform.deltaScale(dscale);
}
glm::vec3 GameObject::deltaScale(float dx, float dy, float dz) {
	this->setModelMatrixDirty();
	return this->transform.deltaScale(dx, dy, dz);
}

glm::vec3 GameObject::getPosition() const {
	return this->transform.getPosition();
}
glm::vec3 GameObject::getRotation() const {
	return this->transform.getRotation();
}
glm::vec3 GameObject::getScale() const {
	return this->transform.getScale();
}

Transform& GameObject::getLocalTransform() {
	return this->transform;
}
void GameObject::clearLocalTransform() {
	this->setModelMatrixDirty();
	this->transform.clear();
}
void GameObject::setLocalMatrix(const glm::mat4& mat) {
	this->transform.fromMatrix(mat);
}
glm::mat4 GameObject::getLocalMatrix() {
	return this->transform.getMatrix();
}

glm::mat4 GameObject::getParentMatrix() {
	if (auto p = this->parent.elevate()) {
		return p->getModelMatrix();
	}
	return glm::mat4(1.0);
}

void GameObject::setModelMatrixDirty() {
	this->modelMatrixDirty = true;
	for (Ref<GameObject>& child : this->children) {
		child->setModelMatrixDirty();
	}
}
glm::mat4 GameObject::getModelMatrix() {
	if (this->modelMatrixDirty) {
		this->modelMatrix = this->getParentMatrix() * this->getLocalMatrix();
	}
	return this->modelMatrix;
}


void GameObject::setParent(Ref<GameObject> parent, bool adjustTransform) {
	// Guaranteed by the Datablock implementation to be non-null.
	Ref<GameObject> selfRef = this->getRef();

	if (auto p = this->parent.elevate()) {
		if (p.get() == parent.get()) {
			// Assigning to the same (non-null) parent; don't change anything.
			return;
		}
		auto& pchildren = p->children;
		pchildren.erase(std::find_if(pchildren.begin(), pchildren.begin(),
			[this](Ref<GameObject>& r) { return r.get() == this; }
		));
	}
	else if (!parent) {
		// Both old and new parent are nullptr.
		return;
	}

	// Do not assign the new parent until after adjusting the transform.
	// This ensures the correct model matrix is returned from this->getModelMatrix().
	if (parent) {
		parent->children.push_back(selfRef);
		if (adjustTransform) {
			glm::mat4 model = this->getModelMatrix();
			model = glm::inverse(parent->getModelMatrix()) * model;
			this->transform.fromMatrix(model);
		}
	}
	else if (adjustTransform) {
		// We're unassigning the current parent. Apply the global transform.
		this->transform.fromMatrix(this->getModelMatrix());
	}
	this->parent = parent.weak();
	// TODO: Cycle detection, at least in some debug mode.
}
void GameObject::clearParent(bool adjustTransform) {
	this->setParent(nullptr, adjustTransform);
}
WeakRef<GameObject> GameObject::getParent() {
	return this->parent;
}

const std::vector<Ref<GameObject>>& GameObject::getChildren() {
	return this->children;
}


bool GameObject::removeComponentFirst() {
	if (!this->components.empty()) {
		delete this->components.front();
		this->components.erase(this->components.begin());
		return true;
	}
	return false;
}
bool GameObject::removeComponentLast() {
	if (!this->components.empty()) {
		delete this->components.back();
		this->components.pop_back();
		return true;
	}
	return false;
}
bool GameObject::removeComponent(Component* component) {
	int i = this->getComponentIndex(component);
	if (i >= 0) {
		delete this->components[i];
		this->components.erase(this->components.begin() + i);
		return true;
	}
	return false;
}
bool GameObject::removeComponent(int index) {
	if (index >= 0 && index < this->components.size()) {
		delete this->components[index];
		this->components.erase(this->components.begin() + index);
		return true;
	}
	return false;
}
void GameObject::clearComponents() {
	for (Component* c : this->components) {
		delete c;
	}
	this->components.clear();
}

Component* GameObject::getComponent(int index) {
	if (index >= 0 && index < this->components.size()) {
		return this->components[index];
	}
	return nullptr;
}
int GameObject::getComponentIndex(Component* component) {
	for (int i = 0; i < this->components.size(); i++) {
		if (this->components[i] == component) {
			return i;
		}
	}
	return -1;
}
const std::vector<Component*>& GameObject::getComponents() {
	return this->components;
}

bool GameObject::swapComponents(Component* a, Component* b) {
	int iA = this->getComponentIndex(a);
	if (iA >= 0) {
		int iB = this->getComponentIndex(b);
		return this->swapComponents(iA, iB);
	}
	return false;
}
bool GameObject::swapComponents(int indexA, int indexB) {
	if (indexA < 0 || indexB < 0 ||
		indexA >= this->components.size() || indexB >= this->components.size()) {
		return false;
	}
	std::swap(this->components[indexA], this->components[indexB]);
	return true;
}
bool GameObject::moveComponent(Component* component, int index) {
	if (index < 0 || index >= this->components.size()) {
		return false;
	}
	int i = this->getComponentIndex(component);
	if (i >= 0) {
		return this->moveComponent(i, index);
	}
	return false;
}
bool GameObject::moveComponent(int srcIndex, int dstIndex) {
	if (srcIndex < 0 || dstIndex < 0 ||
		srcIndex >= this->components.size() || dstIndex >= this->components.size()) {
		return false;
	}
	if (srcIndex < dstIndex) {
		std::rotate(
			this->components.begin() + srcIndex,
			this->components.begin() + srcIndex + 1,
			this->components.begin() + dstIndex + 1
		);
	}
	else if (dstIndex > srcIndex) {
		std::rotate(
			this->components.begin() + dstIndex,
			this->components.begin() + srcIndex,
			this->components.begin() + srcIndex + 1
		);
	}
	return true;
}
bool GameObject::moveComponentToFirst(Component* component) {
	int i = this->getComponentIndex(component);
	if (i >= 0) {
		return this->moveComponentToFirst(i);
	}
	return false;
}
bool GameObject::moveComponentToFirst(int index) {
	return this->moveComponent(index, 0);
}
bool GameObject::moveComponentToLast(Component* component) {
	int i = this->getComponentIndex(component);
	if (i >= 0) {
		return this->moveComponentToLast(i);
	}
	return false;
}
bool GameObject::moveComponentToLast(int index) {
	return this->moveComponent(index, (int)this->components.size() - 1);
}


void GameObject::draw() {
	// TODO: Perhaps a default axes render for some debug mode?
}
