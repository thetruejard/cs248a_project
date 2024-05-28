#pragma once
#include "graphics/pipeline/rp_deferred.h"
#include "graphics/graphics_opengl.h"


class RP_Deferred_OpenGL : public RP_Deferred {
public:

	RP_Deferred_OpenGL(Graphics& graphics);

	virtual void init();

	virtual void resizeFramebuffer(size_t width, size_t height) override;

	virtual void render(Scene* scene) override;

	virtual void renderMesh(Mesh* mesh) override;

	virtual void renderPrimitive(Rectangle rect,
		Ref<Material> material) override;

private:

	Shader_OpenGL gBufferShader;
	Shader_OpenGL lightShader;
	Shader_OpenGL rawShader;

	GLsizei width = 0;
	GLsizei height = 0;

	GLuint gBuffer = 0;
	GLuint gbAlbedoTex = 0;
	GLuint gbPosTex = 0;
	GLuint gbNormalTex = 0;
	GLuint gbMetalRoughTex = 0;
	GLuint gbDepthRB = 0;

	// TEMP while waiting for gamma solution.
	GLuint preGammaFBO = 0;
	GLuint preGammaTex = 0;

};