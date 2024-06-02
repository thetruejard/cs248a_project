#pragma once
#include "graphics/pipeline/rp_none.h"
#include "graphics/graphics_opengl.h"
#include "geometry/sphere.h"


class RP_None_OpenGL : public RP_None {
public:

	RP_None_OpenGL(Graphics& graphics);

	virtual void init();

	virtual void resizeFramebuffer(size_t width, size_t height) override;

	virtual void render(Scene* scene) override;

	virtual void renderMesh(Mesh* mesh) override;

	virtual void renderPrimitive(Rectangle rect,
		Ref<Material> material) override;


};