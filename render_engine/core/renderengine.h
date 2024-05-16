#pragma once
#include "core/datablock.h"
#include "core/scene.h"
#include "depsgraph/depsgraph.h"
#include "graphics/graphics.h"
#include "graphics/texture.h"
#include "io/inputcontext.h"

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>


/*
* RenderEngine is the core class for the game engine.
* This class keeps record of everything, including state, windows, the current
* scene, threads, resources, etc.
* 
* In C++, the outline of classes are defined separately from their implementations.
* See RenderEngine.cpp for the implementation of this class (i.e. function definitions).
*/
class RenderEngine {
public:

	RenderEngine();
	RenderEngine(Graphics::Backend backend);
	RenderEngine(const RenderEngine&) = delete;
	RenderEngine(RenderEngine&&) = delete;
	RenderEngine& operator=(const RenderEngine&) = delete;
	RenderEngine& operator=(RenderEngine&&) = delete;
	~RenderEngine();

	/*
	* Starts the game engine. Does not return until the game engine exits.
	* If fullscreen is true, the width and height parameters are ignored.
	* TODO: Support an icon for the window (a game logo?)
	*/
	void launch(
		std::string windowTitle,
		size_t width,
		size_t height,
		bool fullscreen
	);

	void launch_eval(
		std::string windowTitle,
		size_t width,
		size_t height,
		bool fullscreen,
		glm::mat4* camMats,
		size_t numCamMats
	);

	Graphics* getGraphics();
	InputContext* getInputContext();

	std::string getWindowTitle();
	void setWindowTitle(std::string title);


	/*
	* ===== SCENES =====
	*/

	/*
	* Creates a new empty scene.
	*/
	Ref<Scene> createScene();

	/*
	* Sets the specified scene as the active scene.
	*/
	void setActiveScene(Ref<Scene> scene);
	Ref<Scene> getActiveScene();


	/*
	* ===== OBJECTS =====
	*/

	/*
	* Creates a new instance of the templated object type.
	*/
	template<typename T, typename... Args>
	Ref<T> createObject(Args... args) {
		Ref<T> g = this->objects.create<T>(this, args...);
		if (g) {
			g->setName(g->getTypeName() + "." + std::to_string(g->getID()));
		}
		return g;
	}

	/*
	* Gets a Ref to an object from its ID.
	*/
	Ref<GameObject> getObjectByID(GameObjectID id);


	
	Ref<Mesh> createMesh();
	Ref<Material> createMaterial();
	Ref<Texture> createTexture();

	Ref<Texture> getTextureByPath(const std::filesystem::path& path);



	Depsgraph* getDepsgraph();



private:


	std::string windowTitle;

	/*
	* This is a pointer to a class instance, meaning the class doesn't exist until we
	* explicitly create it with the "new" keyword.
	* We'll create it when we launch the engine, and delete it when we shut down. We
	* *need* to make sure it's deleted eventually or we'll have a memory leak.
	* The advantage of using a pointer here is that we can use a virtual class; like
	* abstract classes in Java, they let us inherit from the base "Graphics" class
	* and override its member functions.
	*/
	Graphics* graphics = nullptr;

	/*
	* The input context manages all user input, including mouse, keyboard, text, and
	* any external devices such as joysticks or third-party game controllers.
	* TODO: For now, InputContext does not support external devices.
	*/
	InputContext inputContext;

	Depsgraph depsgraph;

	/*
	* Datablock Managers.
	* These containers help maintain datablock IDs, manage memory (de)allocation, and
	* enable lazy garbage collection. Keeping these datablocks in centralized
	* containers also helps simplify multi-use of datablocks, such as textures that
	* appear in multiple meshes, or instancing meshes across multiple objects/scenes.
	*/
	DatablockManager<Scene> scenes;
	DatablockManager<GameObject> objects;
	DatablockManager<Mesh> meshes;
	DatablockManager<Material> materials;
	DatablockManager<Texture> textures;

	/*
	* ===== Current Context Information =====
	*/

	Ref<Scene> activeScene = nullptr;


};
