#pragma once
#include "graphics/pipeline/rp_clay.h"
#include "graphics/graphics_opengl.h"


class RP_Clay_OpenGL : public RP_Clay {
public:

	RP_Clay_OpenGL(Graphics& graphics);

	virtual void init() override;
	
	virtual void render(Scene* scene) override;

	virtual void renderMesh(Mesh* mesh) override;

private:

	Shader_OpenGL clayShader;

};