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


	// Indices should align with values in deferred_light.frag.
	enum class LightCulling : GLint {
		None = 0,
		BoundingSphere = 1,
		RasterSphere = 2,
		Tiled = 3,
		Clustered = 4,
	};
	LightCulling culling = LightCulling::None;


private:

	Shader_OpenGL gBufferShader;
	Shader_OpenGL lightShader;
	Shader_OpenGL rawShader;
	Shader_OpenGL postShader;

	GLsizei width = 0;
	GLsizei height = 0;

	GLuint gBuffer = 0;
	GLuint gbAlbedoTex = 0;
	GLuint gbPosTex = 0;
	GLuint gbNormalTex = 0;
	GLuint gbMetalRoughTex = 0;
	GLuint gbDepthRB = 0;

	GLuint postFBO = 0;
	GLuint postTex = 0;

	GLuint lightsSSBO = 0;
	size_t lightsSSBONumLights = 0;
	static constexpr GLuint lightsSSBOBinding = 0;		// Must align with deferred_light.frag
	void updateLightsSSBO(Scene* scene, glm::mat4 viewMatrix);

};