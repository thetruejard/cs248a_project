#pragma once
#include "geometry/rectangle.h"
#include "graphics/Mesh.h"

#include <string>

class Graphics;
class Scene;


/*
* Render pipelines master list.
* It's possible that not every graphics backend supports every render pipeline,
* or that some pipeline/backend combinations may have incomplete features.
*/
enum class RenderPipelineType {
	None,
	Clay,
	Temp,
	Deferred,
	Forward,
};
inline constexpr RenderPipelineType DefaultRenderPipeline = RenderPipelineType::Deferred;


/*
* An abstract RenderPipeline class.
* All pipeline types inherit from this class. Note that a derived class represents a
* pipeline TYPE, not an implementation. Implementations are further derived from the
* type classes and are backend-specific.
* Ex: RenderPipeline (abstract) -> RP_Clay (abstract) -> RP_Clay_OpenGL (implementation).
*/
class RenderPipeline {
public:

	RenderPipeline(Graphics& graphics);

	virtual RenderPipelineType getType() = 0;
	virtual std::string getName() = 0;

	/*
	* Called once when the engine is launched. resizeFramebuffer() is also called seperately
	* after this function before any rendering starts.
	*/
	virtual void init();

	/*
	* Called any time the framebuffer resizes, and once right after init().
	*/
	virtual void resizeFramebuffer(size_t width, size_t height);

	virtual void render(Scene* scene) = 0;

	virtual void renderMesh(Mesh* mesh) = 0;
	virtual void renderPrimitive(Rectangle rect, Ref<Material> material);

protected:

	Graphics* thisGraphics;

};