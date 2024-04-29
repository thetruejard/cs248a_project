#pragma once
#include "graphics/pipeline/rp_temp.h"
#include "graphics/graphics_opengl.h"


class RP_Temp_OpenGL : public RP_Temp {
public:

	RP_Temp_OpenGL(Graphics& graphics);

	virtual void init() override;

	virtual void render(Scene* scene) override;

	virtual void renderMesh(Mesh* mesh) override;

private:

	Shader_OpenGL tempShader;

};