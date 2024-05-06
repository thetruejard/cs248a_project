#pragma once
#include "geometry/rectangle.h"
#include "graphics/mesh.h"
#include "graphics/pipeline/renderpipeline.h"

#include "glm/glm.hpp"

#include <string>


struct GLFWwindow;
class GPUMesh;


/*
* The primary class for the graphics engine.
* This is a virtual class, so other classes can inherit from it to support
* different GPU backends such as OpenGL, Vulkan, DirectX, etc.
*/
class Graphics {
public:

	enum class Backend {
		NONE,
		OPENGL,
		// TODO: We can later choose to support other backends such as Vulkan or DirectX.
	};

	virtual ~Graphics();

	/*
	* Returns the ideal backend for this platform.
	* Currently, only OpenGL 4.0+ is supported.
	*/
	static Graphics::Backend getPreferredBackend();

	/*
	* Returns a new Graphics instance using the given backend, or nullptr on error.
	* If the given backend is NONE, the preferred backend for this platform is used.
	*/
	static Graphics* openGraphics(RenderEngine* engine, Graphics::Backend backend);

	/*
	* Returns the enum associated with the backend of this Graphics instance.
	*/
	Graphics::Backend getBackend();

	/*
	* Returns the name of the backend of this Graphics instance. This may be more
	* specific than the enum, i.e. it may include a version number.
	*/
	virtual std::string getBackendString();

	/*
	* Returns a string of the name/vendor/model of the graphics card in use.
	*/
	virtual std::string getGPUNameString();

	/*
	* Sets the render pipeline to be used for this Graphics instance.
	* This resets all pipeline settings to their default values.
	* In most cases, this will be called once at game startup.
	* A default pipeline may be selected by createWindow(), see its definition.
	* TODO: Support editing pipeline settings.
	*/
	virtual void setRenderPipeline(RenderPipelineType pipelineType) = 0;

	/*
	* Called by Callbacks_GLFW when the size of the primary framebuffer changes.
	*/
	virtual void resizeFramebuffer(size_t width, size_t height) = 0;


	/*
	* GPU Object Types.
	*/

	virtual GPUMesh* createMesh() = 0;
	virtual GPUTexture* createTexture(Texture* thisTexture) = 0;



	/*
	* Creates a window in the center of the default monitor.
	* If fullscreen is true, the width and height parameters are ignored.
	* TODO: Support an icon for the window (a game logo?)
	* Returns true on success or false on error.
	* 
	* If a render pipeline has not been selected with setRenderPipeline() before
	* this call, DefaultRenderPipeline is selected.
	* 
	* The "virtual" keyword makes it so classes that inherit from this class can
	* override this method.
	*/
	virtual bool createWindow(
		std::string window_title,
		size_t width,
		size_t height,
		bool fullscreen
	) = 0;

	/*
	* Destroys the window created by createWindow. If no such window exists, does nothing.
	*/
	virtual void destroyWindow() = 0;

	/*
	* Returns the pointer to the GLFW window.
	*/
	GLFWwindow* getWindow();

	/*
	* Polls events from the window and delegates them to event callbacks in the engine.
	* Returns false when the main event loop should end, e.g. the red X is clicked.
	*/
	virtual bool pollEvents() = 0;

	/*
	* All backends use double-buffered rendering.
	* Pipelines use this function to swap buffers when done drawing.
	*/
	virtual void swapBuffers() = 0;

	/*
	* Render the given scene to the window.
	*/
	void render(Scene* scene);

	/*
	* Renders the given entity using the current render pipeline.
	*/
	void renderMesh(Mesh* mesh);
	void renderPrimitive(Rectangle rect, Ref<Material> material);



	/*
	* Primitive meshes.
	* 2D primitives are flat on the plane z=0.
	*/
	struct _Primitives {
		// Rectangle: [0,1]x[0,1]
		Ref<Mesh> rectangle;
		// Circle: Unit circle at (0,0)
		//Ref<Mesh> circle;
		// Cube: [0,1]x[0,1]x[0,1]
		//Ref<Mesh> cube;
		// Sphere: Unit sphere at (0,0,0)
		//Ref<Mesh> sphere;
	} primitives;

	void initPrimitives();


protected:

	RenderEngine* thisEngine;

	/*
	* The current backend for this Graphics instance.
	* Assigned in the constructor of derived classes.
	*/
	Graphics::Backend backend = Graphics::Backend::NONE;

	/*
	* The render pipeline to be used by this backend.
	*/
	RenderPipeline* pipeline = nullptr;

	/*
	* The GLFW window.
	*/
	GLFWwindow* window = nullptr;


};


/*
* A backend-specific class that holds relevant GPU data representing a mesh.
* Each backend implements its own version of this class.
*/
class GPUMesh {
public:

	virtual ~GPUMesh() = default;

	virtual bool uploadFrom(const Mesh& mesh) = 0;

	virtual void draw() = 0;

};


/*
* A backend-specific class that holds relevant GPU data representing a texture.
* Each backend implements its own version of this class.
*/
class GPUTexture {
public:

	GPUTexture(Texture* thisTexture);
	virtual ~GPUTexture() = default;

	virtual bool upload(
		uint8_t* data,
		size_t width,
		size_t height,
		size_t numChannels
	) = 0;


protected:

	Texture* thisTexture;

};