#pragma once
#include "core/datablock.h"
#include "objects/gameobject.h"
#include "objects/go_light.h"
#include "objects/go_camera.h"

DATABLOCK_ID(Scene);

class CppGameEngine;


/*
* Refs:
* WeakRefs:
*	- Each GameObject in the scene has a WeakRef pointing to the scene it belongs to.
*/

class Scene : public Datablock {
public:

	Scene(SceneID id, CppGameEngine* engine);

	/*
	* Adds the given object just below the root of the scene WITHOUT copying.
	* This effectively adds the entire heirarchy below this object to the scene.
	* Fails if the given object already has a parent, in which case it belongs
	* to the same scene as its parent. Returns whether it succeeded.
	*/
	bool addObject(Ref<GameObject> object);

	/*
	* Gets the root of the scene.
	* The root is always a plain GameObject with no transformation.
	*/
	Ref<GameObject> getRoot();

	/*
	* The active camera is the camera used for rendering to the primary display.
	*/
	void setActiveCamera(Ref<GO_Camera> camera);
	Ref<GO_Camera> getActiveCamera();


	// TEMP
	std::vector<GO_Light*> lights;

	/*
	* Evaluates all object components in this scene.
	*/
	void evaluateComponents(float deltaTime);


	CppGameEngine* getEngine();


	// TEMP
	glm::vec3 backgroundColor;



private:

	CppGameEngine* thisEngine;

	Ref<GameObject> root;

	WeakRef<GO_Camera> activeCamera;
	
};