#pragma once
#include "graphics/pipeline/rp_forward.h"
#include "graphics/graphics_opengl.h"
#include "geometry/sphere.h"


class RP_Forward_OpenGL : public RP_Forward {
public:

	RP_Forward_OpenGL(Graphics& graphics);

	virtual void init();

	virtual void resizeFramebuffer(size_t width, size_t height) override;

	virtual void render(Scene* scene) override;

	virtual void renderMesh(Mesh* mesh) override;

	virtual void renderPrimitive(Rectangle rect,
		Ref<Material> material) override;


	// Indices should align with values in forward.frag.
	enum class LightCulling : GLint {
		None = 0,
		BoundingSphere = 1,
		TiledCPU = 3,
		ClusteredCPU = 4,
		TiledGPU = 5,
		ClusteredGPU = 6,
	};
	LightCulling culling = LightCulling::None;
	// (X,Y,Z) For tiled (instead of clustered), third element should be 1.
	glm::ivec3 numTiles = glm::ivec3(80, 45, 32);
	GLint maxLightsPerTile = 64;


private:

	Shader_OpenGL forwardShader;
	Shader_OpenGL rawShader;
	Shader_OpenGL postShader;
	Shader_OpenGL zprepassShader;

	GLsizei width = 0;
	GLsizei height = 0;


	GLuint postFBO = 0;
	GLuint postTex = 0;
	GLuint postDepthRB = 0;

	GLuint lightsSSBO = 0;
	size_t lightsSSBONumLights = 0;
	static constexpr GLuint lightsSSBOBinding = 0;		// Must align with deferred_light.frag
	void updateLightsSSBO(Scene* scene, glm::mat4 viewMatrix);


	// The SSBO storing mappings to ranges in lightsIndexSSBO (2 values per cluster, pos and len)
	GLuint tileLightMappingSSBO = 0;
	glm::ivec3 tileLightMappingRes;			// The resolution allocated. For tiles, z=1.
	std::vector<GLint> tileLightMapping;	// When using CPU, stores values to be copied into SSBO.
	static constexpr GLuint tileLightMappingSSBOBinding = 1;		// Must align with deferred_light.frag
	void updateTileLightMappingSSBO();		// Checks size and, if CPU, copies values from tileLightMapping.


	// The SSBO containing light lists for each cluster. tileLightMapping stores ranges in this list.
	GLuint lightsIndexSSBO = 0;
	size_t lightsIndexSSBOSize = 0;			// Size in bytes.
	static constexpr GLuint lightsIndexSSBOBinding = 2;				// Must align with deferred_light.frag
	std::vector<GLint> lightsIndex;			// When using CPU, stores values to be copied into SSBO.
	GLuint globalIndexCountSSBO = 0;
	static constexpr GLuint globalIndexCountSSBOBinding = 4;
	void updateLightsIndexSSBO();			// Checks size and, if CPU, copies values from lightsIndex.


	void runTilesCPU(Scene* scene);
	void runClustersCPU(Scene* scene);

	// Cache light volumes to avoid reallocating memory.
	std::vector<std::pair<Sphere, float>> lightVolumes;




	GLuint clustersSSBO = 0;
	glm::ivec3 clustersRes;			// The resolution allocated. For tiles, z=1.
	//std::vector<glm::vec4> clusters;	// Each cluster has 2 elements, min/max AABBs.
	static constexpr GLuint clustersSSBOBinding = 3;		// Must align with deferred_light.frag
	Shader_OpenGL clusterGenShader;
	void updateClustersSSBO(Scene* scene);


	Shader_OpenGL clusterCullLightsShader;

	void runClustersGPU(Scene* scene);
};