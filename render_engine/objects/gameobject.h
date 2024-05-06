#pragma once
#include "components/component.h"
#include "core/datablock.h"
#include "core/transform.h"

#include <string>
#include <vector>


DATABLOCK_ID(GameObject);

class RenderEngine;
class Scene;


/*
* GameObjects are datablocks representing elements of a scene.
* 
* Reference-counted Refs are used in the following places:
*	1. In the scene heirarchy, objects store a Ref to each of their children. They
*		store a WeakRef to their parent. Heirarchy information is managed by the
*		owning Scene class.
*	2. When importing assets, a heirachy of Refs is created and the root is returned
*		to the caller. It is the caller's responsibility to persist this Ref and/or
*		add it to a scene, otherwise it will be deleted.
*/

class GameObject : public Datablock {
public:

	GameObject(GameObjectID id, RenderEngine* engine);
	virtual ~GameObject();
	virtual std::string getTypeName();

	std::string getName();
	void setName(std::string name);

	// Sets the scene this object is assigned to. Pass nullptr to only unassign.
	// If you add the object to a scene through the scene's interface, such as
	// with scene->addObject(), then this function is called automatically.
	// If overridden, the derived class MUST call this parent method.
	virtual void setScene(Ref<Scene> scene);

	WeakRef<Scene> getScene();
	
	/*
	* Transform. See transform.h.
	*/
	
	void setPosition(glm::vec3 position);
	void setPosition(float x, float y, float z);
	void setRotation(glm::vec3 euler);
	void setRotation(float yaw, float pitch, float roll);
	void setScale(glm::vec3 scale);
	void setScale(float x, float y, float z);

	glm::vec3 deltaPosition(glm::vec3 dposition);
	glm::vec3 deltaPosition(float dx, float dy, float dz);
	glm::vec3 deltaPositionWithRot(glm::vec3 dposition);
	glm::vec3 deltaPositionWithYawPitch(glm::vec3 dposition);
	glm::vec3 deltaPositionWithYaw(glm::vec3 dposition);
	glm::vec3 deltaRotation(glm::vec3 deuler);
	glm::vec3 deltaRotation(float dyaw, float dpitch, float droll);
	glm::vec3 deltaScale(glm::vec3 dscale);
	glm::vec3 deltaScale(float dx, float dy, float dz);

	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	glm::vec3 getScale() const;

	Transform& getLocalTransform();
	void clearLocalTransform();
	void setLocalMatrix(const glm::mat4& mat);
	glm::mat4 getLocalMatrix();

	glm::mat4 getParentMatrix();

	// Recursively sets the modelMatrixDirty flag for all children.
	virtual void setModelMatrixDirty();
	// The model matrix is the final matrix used for the display of the object.
	// Model = Parent * Local
	glm::mat4 getModelMatrix();


	/*
	* Relations.
	*/

	// If adjustTransform is true, the object's local-space transform is modified to
	// keep the same world-space transform.
	// If false, the object's local transform will not be modified.
	void setParent(Ref<GameObject> parent, bool adjustTransform);
	// Equivalent to setParent(nullptr, adjustTransform).
	void clearParent(bool adjustTransform);
	WeakRef<GameObject> getParent();
	
	// DO NOT MODIFY THE RETURNED VECTOR. It is for iteration only.
	const std::vector<Ref<GameObject>>& getChildren();


	/*
	* Components.
	*/

	// Addition.
	// Returns the added component, or nullptr on failure.
	template<typename Type, typename... Args>
	Type* addComponentFirst(Args... args) {
		this->addComponent(0, args);
	}
	template<typename Type, typename... Args>
	Type* addComponent(Args... args) {
		Type* newC = new Type(this, args...);
		if (newC) {
			this->components.push_back(newC);
		}
		return newC;
	}
	template<typename Type, typename... Args>
	Type* addComponentIndex(int index, Args... args) {
		if (index < 0 || index > this->components.size()) {
			return nullptr;
		}
		Type* newC = new Type(this, args...);
		if (newC) {
			this->components.insert(this->components.begin() + index, newC);
		}
		return newC;
	}

	// Removal.
	// Returns whether succeeded.
	bool removeComponentFirst();
	bool removeComponentLast();
	template<typename Type>
	bool removeComponent() {
		int i = this->getComponentIndex<Type>();
		if (i >= 0) {
			this->components.erase(this->components.begin() + i);
			return true;
		}
		return false;
	}
	bool removeComponent(Component* component);
	bool removeComponent(int index);
	void clearComponents();

	// Retrieval.
	// If there are multiple matches, the first found is returned.
	// If no match is found, returns nullptr (pointer) or -1 (index).
	template<typename Type>
	Type* getComponent() {
		for (Component* c : this->components) {
			Type* t = c->isType<Type>();
			if (t) {
				return t;
			}
		}
		return nullptr;
	}
	template<typename Type>
	int getComponentIndex() {
		for (int i = 0; i < this->components.size(); i++) {
			if (this->components[i]->isType<Type>()) {
				return i;
			}
		}
		return -1;
	}
	Component* getComponent(int index);
	int getComponentIndex(Component* component);
	const std::vector<Component*>& getComponents();

	// Rearrangement.
	// Returns whether succeeded.
	bool swapComponents(Component* a, Component* b);
	bool swapComponents(int indexA, int indexB);
	bool moveComponent(Component* component, int index);
	bool moveComponent(int srcIndex, int dstIndex);
	bool moveComponentToFirst(Component* component);
	bool moveComponentToFirst(int index);
	bool moveComponentToLast(Component* component);
	bool moveComponentToLast(int index);


	/*
	* Render.
	*/
	virtual void draw();


protected:

	std::string name;

	WeakRef<Scene> thisScene = nullptr;

	/*
	* For the purpose of saving compute time, the "hasTransform" flag indicates whether
	* this object has a local transform. If false, the transform for this object is
	* identical to its parent, or the identity matrix if no parent exists.
	* TODO: Allow setting hasTransform.
	*/
	bool hasTransform = true;
	Transform transform;

	/*
	* The final model matrix for this object, after the scene graph is fully resolved.
	* This matrix is cached and only recomputed when necessary; hence the dirty flag.
	* setModelMatrixDirty() recursively sets the dirty flag of all children too.
	* Use getModelMatrix() to check the dirty flag and recompute if needed.
	*/
	bool modelMatrixDirty = true;
	glm::mat4 modelMatrix = glm::mat4(1.0f);


	WeakRef<GameObject> parent = nullptr;
	std::vector<Ref<GameObject>> children;


	std::vector<Component*> components;


};
