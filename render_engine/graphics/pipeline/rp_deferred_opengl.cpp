#include "graphics/pipeline/rp_deferred_opengl.h"
#include "core/scene.h"
#include "objects/gameobject.h"
#include "objects/go_camera.h"
#include "utils/printutils.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <sstream>
#include <fstream>



RP_Deferred_OpenGL::RP_Deferred_OpenGL(Graphics& graphics) : RP_Deferred(graphics) {}


void RP_Deferred_OpenGL::init() {

	std::cout << "RP_Deferred_OpenGL::init()\n";

	this->gBufferShader.read(
		"shaders/opengl/deferred_gbuffer.vert", 
		"shaders/opengl/deferred_gbuffer.frag"
	);
	this->lightShader.read(
		"shaders/opengl/deferred_light.vert",
		"shaders/opengl/deferred_light.frag"
	);
	this->rawShader.read(
		"shaders/opengl/raw.vert",
		"shaders/opengl/raw.frag"
	);
	this->postShader.read(
		"shaders/opengl/post.vert",
		"shaders/opengl/post.frag"
	);
}

void RP_Deferred_OpenGL::resizeFramebuffer(size_t width, size_t height) {

	std::cout << "RP_Deferred_OpenGL::resizeFramebuffer(" << width << ", " << height << ")\n";

	if (this->width == (GLsizei)width && this->height == (GLsizei)height) {
		return;
	}

	// Delete any existing gBuffer components.
	if (glIsFramebuffer(this->gBuffer)) {
		glDeleteFramebuffers(1, &this->gBuffer);
	}
	if (glIsTexture(this->gbPosTex)) {
		glDeleteTextures(1, &this->gbPosTex);
	}
	if (glIsTexture(this->gbNormalTex)) {
		glDeleteTextures(1, &this->gbNormalTex);
	}
	if (glIsTexture(this->gbAlbedoTex)) {
		glDeleteTextures(1, &this->gbAlbedoTex);
	}
	if (glIsTexture(this->gbMetalRoughTex)) {
		glDeleteTextures(1, &this->gbMetalRoughTex);
	}
	if (glIsRenderbuffer(this->gbDepthRB)) {
		glDeleteRenderbuffers(1, &this->gbDepthRB);
	}
	
	this->width = (GLsizei)width;
	this->height = (GLsizei)height;

	// Prepare the gBuffer.
	glGenFramebuffers(1, &this->gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);

	// Position texture.
	glGenTextures(1, &this->gbPosTex);
	glBindTexture(GL_TEXTURE_2D, this->gbPosTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, this->width, this->height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gbPosTex, 0);
	
	// Normal texture.
	glGenTextures(1, &this->gbNormalTex);
	glBindTexture(GL_TEXTURE_2D, this->gbNormalTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, this->width, this->height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->gbNormalTex, 0);

	// Color texture.
	glGenTextures(1, &this->gbAlbedoTex);
	glBindTexture(GL_TEXTURE_2D, this->gbAlbedoTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, this->gbAlbedoTex, 0);

	// Metalness/Roughness texture.
	glGenTextures(1, &this->gbMetalRoughTex);
	glBindTexture(GL_TEXTURE_2D, this->gbMetalRoughTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, this->width, this->height, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, this->gbMetalRoughTex, 0);

	// Tell OpenGL which color attachments we'll use for rendering (specific to this framebuffer).
	GLenum attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	// Create and attach the depth buffer.
	glGenRenderbuffers(1, &this->gbDepthRB);
	glBindRenderbuffer(GL_RENDERBUFFER, this->gbDepthRB);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->gbDepthRB);

	// Confirm completeness.
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Deferred renderer: Failed to initialize the gBuffer.\n";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// TEMP: until a gamma solution

	if (glIsFramebuffer(this->postFBO)) {
		glDeleteFramebuffers(1, &this->postFBO);
	}
	if (glIsTexture(this->postTex)) {
		glDeleteTextures(1, &this->postTex);
	}
	glGenFramebuffers(1, &this->postFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, this->postFBO);
	glGenTextures(1, &this->postTex);
	glBindTexture(GL_TEXTURE_2D, this->postTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->postTex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Deferred renderer: Failed to initialize preGamma buffer.\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


static void bindMaterial(Shader_OpenGL& shader, Ref<Material> material) {
	// TODO: Support binding different types/more complex materials.
	if (material) {
		// Diffuse tex/color
		shader.setUniform4f("colorDiffuse", material->getDiffuseColor());
		if (material->getDiffuseTexture()) {
			GPUTexture* gpuTex = material->getDiffuseTexture()->getGPUTexture();
			shader.setUniformTex("textureDiffuse",
				((GPUTexture_OpenGL*)gpuTex)->getGLTexID(),
				0, GL_TEXTURE_2D
			);
		}
		// Metalness
		shader.setUniform2f("metalnessFac", glm::vec2(material->getMetalness(),
			1.0f-(float)bool(material->getMetalnessTexture())));
		if (material->getMetalnessTexture()) {
			GPUTexture* gpuTex = material->getMetalnessTexture()->getGPUTexture();
			shader.setUniformTex("textureMetalness",
				((GPUTexture_OpenGL*)gpuTex)->getGLTexID(),
				1, GL_TEXTURE_2D
			);
		}
		// Roughness
		shader.setUniform2f("roughnessFac", glm::vec2(material->getRoughness(),
			1.0f - (float)bool(material->getRoughnessTexture())));
		if (material->getRoughnessTexture()) {
			GPUTexture* gpuTex = material->getRoughnessTexture()->getGPUTexture();
			shader.setUniformTex("textureRoughness",
				((GPUTexture_OpenGL*)gpuTex)->getGLTexID(),
				2, GL_TEXTURE_2D
			);
		}
		// Normal
		shader.setUniform1i("useNormalTex", (GLint)bool(material->getNormalTexture()));
		if (material->getNormalTexture()) {
			GPUTexture* gpuTex = material->getNormalTexture()->getGPUTexture();
			shader.setUniformTex("textureNormal",
				((GPUTexture_OpenGL*)gpuTex)->getGLTexID(),
				3, GL_TEXTURE_2D
			);
		}

		// TODO: TEMP
		if (material->wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	else {
		shader.setUniform4f("colorDiffuse", glm::vec4(1.0f));
		shader.setUniformTex("textureDiffuse",
			0, 0, GL_TEXTURE_2D
		);
	}
}

static void renderSubtree(
	Shader_OpenGL& shader, GameObject* obj,
	const glm::mat4& viewMat, const glm::mat4 projMat
) {

	glm::mat4 mvMat = viewMat * obj->getModelMatrix();
	glm::mat4 mvpMat = projMat * mvMat;
	shader.setUniformMat4("mvMat", mvMat);
	shader.setUniformMat4("normalMat", glm::inverse(glm::transpose(mvMat)));
	shader.setUniformMat4("mvpMat", mvpMat);

	obj->draw();

	for (auto& child : obj->getChildren()) {
		renderSubtree(shader, child.get(), viewMat, projMat);
	}
}

void RP_Deferred_OpenGL::render(Scene* scene) {

	// Pass 1: Render to gBuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	this->gBufferShader.bind();

	Ref<GO_Camera> activeCamera = scene->getActiveCamera();
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	if (activeCamera) {
		viewMatrix = activeCamera->getViewMatrix();
		projMatrix = activeCamera->getProjectionMatrix();
	}
	else {
		viewMatrix = glm::mat4(1.0f);
		projMatrix = glm::mat4(1.0f);
	}

	renderSubtree(this->gBufferShader, scene->getRoot().get(), viewMatrix, projMatrix);


	// Pass 2: Render lights.
	//this->lightShader.bind();
	//this->lightShader.setUniform1f("specularShininess", 6.0f);
	//this->lightShader.setUniform1f("specular", 1.0f);


	glBindFramebuffer(GL_FRAMEBUFFER, this->postFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//this->rawShader.bind();
	//this->rawShader.setUniformTex("textureMain", this->gbPosTex, 0);
	//this->renderPrimitive(Rectangle(-1.0f, 0.0f, 1.0f, 1.0f), nullptr);
	//this->rawShader.setUniformTex("textureMain", this->gbNormalTex, 0);
	//this->renderPrimitive(Rectangle(0.0f, 0.0f, 1.0f, 1.0f), nullptr);
	//this->rawShader.setUniformTex("textureMain", this->gbAlbedoTex, 0);
	//this->renderPrimitive(Rectangle(-1.0f, -1.0f, 1.0f, 1.0f), nullptr);


	this->lightShader.bind();
	this->lightShader.setUniform2f("viewportSize", glm::vec2((float)this->width, (float)this->height));
	this->lightShader.setUniform3f("numTiles", glm::vec3(this->numTiles));

	// Fullscreen quad matrix.
	glm::mat4 mat;
	mat[0] = glm::vec4(2.0f, 0.0f, 0.0f, 0.0f);
	mat[1] = glm::vec4(0.0f, 2.0f, 0.0f, 0.0f);
	mat[2] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	mat[3] = glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
	this->lightShader.setUniformMat4("mat", mat);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);


	this->lightShader.setUniformTex("texturePos", this->gbPosTex, 0);
	this->lightShader.setUniformTex("textureNormals", this->gbNormalTex, 1);
	this->lightShader.setUniformTex("textureAlbedo", this->gbAlbedoTex, 2);
	this->lightShader.setUniformTex("textureMetalRough", this->gbMetalRoughTex, 3);

	
	this->updateLightsSSBO(scene, viewMatrix);


	if (this->culling == LightCulling::TiledCPU) {
		this->runTilesCPU(scene);
	}
	else if (this->culling == LightCulling::ClusteredCPU) {
		this->runClustersCPU(scene);
	}
	else if (this->culling == LightCulling::TiledGPU) {

	}
	else if (this->culling == LightCulling::ClusteredGPU) {
		this->runClustersGPU(scene);
	}

	this->lightShader.bind();


	if (this->culling != LightCulling::RasterSphere) {

		this->lightShader.setUniform1f("zNear", scene->getActiveCamera()->projectionParams.perspective.near);
		this->lightShader.setUniform1f("zFar", scene->getActiveCamera()->projectionParams.perspective.far);
		this->lightShader.setUniform2i("cullingMethod", glm::ivec2((GLint)this->culling, 0));
		this->thisGraphics->primitives.rectangle->draw();

	}
	else {
		// Raster sphere
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);	// The sphere's faces point inwards, so cull the outside faces
		// TODO: Make this a sphere
		for (size_t i = 0; i < scene->lights.size(); i++) {
			GO_Light* light = scene->lights[i];
			// (method, light_index)
			this->lightShader.setUniform2i("cullingMethod", glm::ivec2((GLint)LightCulling::RasterSphere, (GLint)i));
			if (light->type == GO_Light::Type::Point) {
				glDepthFunc(GL_GEQUAL);
				glEnable(GL_DEPTH_TEST);
				Sphere bs = light->getBoundingSphere();
				// Rasterize spheres
				glm::mat4 mat;
				mat[0] = glm::vec4(bs.radius, 0.0f, 0.0f, 0.0f);
				mat[1] = glm::vec4(0.0f, bs.radius, 0.0f, 0.0f);
				mat[2] = glm::vec4(0.0f, 0.0f, bs.radius, 0.0f);
				mat[3] = glm::vec4(bs.position, 1.0f);
				mat = projMatrix * viewMatrix * mat;
				this->lightShader.setUniformMat4("mat", mat);
				this->thisGraphics->primitives.sphere->draw();
				glDepthFunc(GL_LEQUAL);
				glDisable(GL_DEPTH_TEST);
			}
			else {
				// Other light type (i.e. sun), render every pixel
				glDisable(GL_CULL_FACE);
				glm::mat4 mat;
				mat[0] = glm::vec4(2.0f, 0.0f, 0.0f, 0.0f);
				mat[1] = glm::vec4(0.0f, 2.0f, 0.0f, 0.0f);
				mat[2] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
				mat[3] = glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
				this->lightShader.setUniformMat4("mat", mat);
				this->thisGraphics->primitives.rectangle->draw();
				glEnable(GL_CULL_FACE);
			}
			
		}
		glDisable(GL_CULL_FACE);

	}

	glDepthMask(GL_TRUE);


	// TEMP: until a gamma solution
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	this->postShader.bind();
	this->postShader.setUniformTex("textureMain", this->postTex);
	mat[0] = glm::vec4(2.0f, 0.0f, 0.0f, 0.0f);
	mat[1] = glm::vec4(0.0f, 2.0f, 0.0f, 0.0f);
	mat[2] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	mat[3] = glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
	this->postShader.setUniformMat4("mat", mat);
	//glEnable(GL_FRAMEBUFFER_SRGB);
	this->thisGraphics->primitives.rectangle->draw();
	//glDisable(GL_FRAMEBUFFER_SRGB);
	// The rest of this is to make sure the background color is drawn.
	glBindFramebuffer(GL_READ_FRAMEBUFFER, this->gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, this->width, this->height, 0, 0, this->width, this->height,
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	mat[3] = glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f);
	this->rawShader.bind();
	this->rawShader.setUniformMat4("mat", mat);
	this->rawShader.setUniform4f("colorMain", glm::vec4(scene->backgroundColor, 1.0f));
	glEnable(GL_DEPTH_TEST);
	this->thisGraphics->primitives.rectangle->draw();


	this->thisGraphics->swapBuffers();
}

void RP_Deferred_OpenGL::renderMesh(Mesh* mesh) {
	GPUMesh* gpuMesh = mesh->getGPUMesh();
	if (gpuMesh) {
		if (mesh->getMaterial()) {
			bindMaterial(this->gBufferShader, mesh->getMaterial());
		}
		gpuMesh->draw();
	}
}

void RP_Deferred_OpenGL::renderPrimitive(
	Rectangle rect, Ref<Material> material
) {
	this->rawShader.bind();
	if (material) {
		this->rawShader.setUniform4f("colorMain", material->getDiffuseColor());
		if (material->getDiffuseTexture()) {
			GPUTexture* gpuTex = material->getDiffuseTexture()->getGPUTexture();
			this->rawShader.setUniformTex("textureMain",
				((GPUTexture_OpenGL*)gpuTex)->getGLTexID(),
				0, GL_TEXTURE_2D
			);
		}
	}
	glm::mat4 mat;
	mat[0] = glm::vec4(rect.widthHeight.x, 0.0f, 0.0f, 0.0f);
	mat[1] = glm::vec4(0.0f, rect.widthHeight.y, 0.0f, 0.0f);
	mat[2] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	mat[3] = glm::vec4(rect.bottomLeft, 0.0f, 1.0f);
	this->rawShader.setUniformMat4("mat", mat);
	glDisable(GL_DEPTH_TEST);
	this->thisGraphics->primitives.rectangle->draw();
}




// We use all vec4s to avoid common alignment bugs in the OpenGL drivers.
struct SSBOLight {
	// Position (xyz) and type (w).
	// Type matches the enum in go_light.h.
	// None=0, Dir=1, Point=2, Spot=3.
	glm::vec4 positionType;			// vec4
	// Normalized direction for point and spot lights.
	glm::vec4 direction;			// vec3
	// Inner & outer angles (radians) for spot lights.
	glm::vec4 innerOuterAngles;		// vec2
	// Color.
	glm::vec4 color;				// vec3
	// Attenuation: (constant, linear, quadratic).
	glm::vec4 attenuation;			// vec3
};

void RP_Deferred_OpenGL::updateLightsSSBO(Scene* scene, glm::mat4 viewMatrix) {
	if (!scene) return;
	std::vector<GO_Light*>& lights = scene->lights;
	if (this->lightsSSBO == 0 || this->lightsSSBONumLights != lights.size()) {
		if (this->lightsSSBO != 0)
			glDeleteBuffers(1, &this->lightsSSBO);
		glGenBuffers(1, &this->lightsSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->lightsSSBO);
		// +4 for ivec4 numLights
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::ivec4) + lights.size() * sizeof(SSBOLight), (void*)0, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->lightsSSBOBinding, this->lightsSSBO);
		this->lightsSSBONumLights = lights.size();
	}
	else {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->lightsSSBO);
	}

	size_t len = sizeof(glm::ivec4) + lights.size() * sizeof(SSBOLight);
	uint8_t* buf = new uint8_t[len];
	// First element is number of lights.
	((glm::ivec4*)buf)[0] = glm::ivec4((GLint)lights.size(), 0, 0, 0);
	// Rest of the array is SSBOLight classes.
	for (size_t i = 0; i < lights.size(); i++) {
		GO_Light* src_light = lights[i];
		SSBOLight* dst_light = ((SSBOLight*)(buf + sizeof(glm::ivec4))) + i;
		glm::vec4 posVector = viewMatrix * src_light->getModelMatrix()[3];
		glm::vec4 dirVector = viewMatrix * glm::vec4(src_light->getWorldSpaceDirection(), 0.0f);
		dst_light->positionType = glm::vec4(glm::vec3(posVector), (float)src_light->type);
		dst_light->direction = glm::vec4(glm::normalize(glm::vec3(dirVector)), 0.0f);
		dst_light->innerOuterAngles = glm::vec4(src_light->innerOuterAngles, 0.0f, 0.0f);
		dst_light->color = glm::vec4(src_light->color, 0.0f);
		dst_light->attenuation = glm::vec4(src_light->attenuation, 0.0f);
	}
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, (GLintptr)0, (GLsizeiptr)len, buf);
	delete[] buf;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


void RP_Deferred_OpenGL::updateTileLightMappingSSBO() {
	if (this->tileLightMappingSSBO == 0 || this->tileLightMappingRes != this->numTiles) {
		if (this->tileLightMappingSSBO != 0)
			glDeleteBuffers(1, &this->tileLightMappingSSBO);
		glGenBuffers(1, &this->tileLightMappingSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->tileLightMappingSSBO);
		GLsizeiptr size = sizeof(GLint) * this->numTiles.x * this->numTiles.y * this->numTiles.z * 2;
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, (void*)0, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->tileLightMappingSSBOBinding, this->tileLightMappingSSBO);
		this->tileLightMappingRes = this->numTiles;
		std::cout << "REALLOCATING tileLightMapping\n";
	}
	else {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->tileLightMappingSSBO);
	}
	if (this->culling == LightCulling::TiledCPU || this->culling == LightCulling::ClusteredCPU) {
		if ((GLint)this->tileLightMapping.size() != this->numTiles.x * this->numTiles.y * this->numTiles.z * 2) {
			std::cout << "TILE LIGHT MAPPING MISMATCH: " << this->tileLightMapping.size() << " | " <<
				this->numTiles.x << "," << this->numTiles.y << "," << this->numTiles.z << "\n";
			return;
		}
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, (GLintptr)0,
			(GLsizeiptr)(sizeof(GLint) * this->tileLightMapping.size()), this->tileLightMapping.data());
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void RP_Deferred_OpenGL::updateLightsIndexSSBO() {
	size_t neededSize;
	if (this->culling == LightCulling::TiledCPU || this->culling == LightCulling::ClusteredCPU)
		neededSize = sizeof(GLint) * this->lightsIndex.size();
	else
		neededSize = sizeof(GLint) * this->maxLightsPerTile * this->numTiles.x * this->numTiles.y * this->numTiles.z;
	if (this->lightsIndexSSBO == 0 || this->lightsIndexSSBOSize < neededSize) {
		if (this->lightsIndexSSBO != 0)
			glDeleteBuffers(1, &this->lightsIndexSSBO);
		glGenBuffers(1, &this->lightsIndexSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->lightsIndexSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)neededSize, (void*)0, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->lightsIndexSSBOBinding, this->lightsIndexSSBO);
		this->lightsIndexSSBOSize = neededSize;
		std::cout << "REALLOCATING lightsIndex (SSBO=" << this->lightsIndexSSBO << ") with size " << neededSize << "\n";
	}
	else {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->lightsIndexSSBO);
	}
	if (this->culling == LightCulling::TiledCPU || this->culling == LightCulling::ClusteredCPU) {
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, (GLintptr)0,
			(GLsizeiptr)(sizeof(GLint) * this->lightsIndex.size()), this->lightsIndex.data());
	}
	if (this->globalIndexCountSSBO == 0) {
		glGenBuffers(1, &this->globalIndexCountSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->globalIndexCountSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)sizeof(GLuint), (void*)0, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->globalIndexCountSSBOBinding, this->globalIndexCountSSBO);
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}



/*
struct Frustum {
	glm::vec3 nearPlaneNormal;
	float nearPlaneDistance;
	glm::vec3 farPlaneNormal;
	float farPlaneDistance;

	Frustum(const glm::mat4& projectionMatrix) {
		glm::vec3 e1, e2, e3;
		glm::mat4 inverseMatrix = glm::inverse(projectionMatrix);

		e1 = glm::vec3(inverseMatrix[3] + inverseMatrix[0], 0.0f, 0.0f) / inverseMatrix[2];
		e2 = glm::vec3(0.0f, inverseMatrix[3] + inverseMatrix[5], 0.0f) / inverseMatrix[7];
		e3 = glm::vec3(inverseMatrix[3] + inverseMatrix[10], inverseMatrix[3] + inverseMatrix[11], 0.0f) / inverseMatrix[14];

		nearPlaneNormal = glm::normalize(glm::cross(e3, e2));
		nearPlaneDistance = -glm::dot(nearPlaneNormal, inverseMatrix[3]);

		farPlaneNormal = glm::normalize(glm::cross(e3, -e2));
		farPlaneDistance = -glm::dot(farPlaneNormal, inverseMatrix[3]);
	}
};*/



static bool lightIntersectsFrustum(GO_Camera* camera, const Sphere& bs, float w,
	glm::vec4 leftBottomRightTop, glm::vec2 nearFar) {
	if (bs.position.z + bs.radius / w < nearFar.x || bs.position.z - bs.radius / w > nearFar.y) {
		return false;
	}
	glm::vec2 radius2D = glm::vec2(
		camera->projectionParams.perspective.aspect, 1.0f)
		* bs.radius / w;
	//return true;
	glm::vec2 pos2D = glm::vec2(bs.position) / w;
	//pos2D = 0.5f * pos2D + 0.5f;
	//float screen_height = 2.0f * tan(camera->projectionParams.perspective.fovy / 2.0f);
	//float screen_width = screen_height * camera->projectionParams.perspective.aspect;
	//glm::vec2 wndSizeAtDepth = viewSpacePos.z * glm::vec2(screen_width, screen_height);
	//glm::vec2 radius2D = glm::vec2(bs.radius) / wndSizeAtDepth;
	//// Just do 2D bounding box comparison
	glm::vec4 lbrt = glm::vec4(pos2D.x - radius2D.x, pos2D.y - radius2D.y, pos2D.x + radius2D.x, pos2D.y + radius2D.y);
	return !(
		lbrt.x >= leftBottomRightTop.z ||
		lbrt.z <= leftBottomRightTop.x ||
		lbrt.y >= leftBottomRightTop.w ||
		lbrt.w <= leftBottomRightTop.y
	);
}



void RP_Deferred_OpenGL::runTilesCPU(Scene* scene) {
	GO_Camera* camera = scene->getActiveCamera().get();
	if (camera == nullptr) {
		return;
	}
	if (tileLightMapping.size() != (size_t)this->numTiles.x * this->numTiles.y * 2)
		tileLightMapping.resize((size_t)this->numTiles.x * this->numTiles.y * 2);
	this->lightsIndex.clear();

	std::vector<GLint> tileLights;
	tileLights.reserve(this->maxLightsPerTile);

	glm::vec2 nearFar = glm::vec2(
		camera->projectionParams.perspective.near,
		camera->projectionParams.perspective.far
	);

	lightVolumes.clear();
	for (GO_Light* light : scene->lights) {
		Sphere s;
		s = light->getBoundingSphere();
		glm::vec4 p = camera->getProjectionMatrix() * camera->getViewMatrix() * glm::vec4(s.position, 1.0f);
		s.position = glm::vec3(p);
		lightVolumes.push_back(std::pair<Sphere, float>(s, p.w));
	}

	// For each tile
	for (size_t y = 0; y < this->numTiles.y; y++) {
		for (size_t x = 0; x < this->numTiles.x; x++) {

			glm::vec4 tileBounds = glm::vec4(
				(float)x / (float)this->numTiles.x,
				(float)y / (float)this->numTiles.y,
				(float)(x + 1) / (float)this->numTiles.x,
				(float)(y + 1) / (float)this->numTiles.y
			);
			tileLights.clear();

			// For each light
			for (GLint i = 0; i < (GLint)scene->lights.size(); i++) {
				GO_Light* light = scene->lights[i];

				// Detect if light intersects tile:
				auto lv = lightVolumes[i];
				if (light->type != GO_Light::Type::Point ||
					lightIntersectsFrustum(camera, lv.first, lv.second, tileBounds, nearFar)) {
					tileLights.push_back(i);
				}

			}

			// Append the tile's lights to the global list and record indices.
			this->tileLightMapping[2 * (y * this->numTiles.x + x)] = (GLint)this->lightsIndex.size();
			this->tileLightMapping[2 * (y * this->numTiles.x + x) + 1] = (GLint)tileLights.size();
			this->lightsIndex.insert(this->lightsIndex.end(), tileLights.begin(), tileLights.end());

		}
	}
	
	this->updateTileLightMappingSSBO();
	this->updateLightsIndexSSBO();

}

void RP_Deferred_OpenGL::runClustersCPU(Scene* scene) {

}






void RP_Deferred_OpenGL::updateClustersSSBO(Scene* scene) {
	GO_Camera* camera = scene->getActiveCamera().get();
	if (!camera)
		return;
	if (this->clustersSSBO == 0 || this->clustersRes != this->numTiles) {
		if (this->clustersSSBO != 0)
			glDeleteBuffers(1, &this->clustersSSBO);
		glGenBuffers(1, &this->clustersSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->clustersSSBO);
		GLsizeiptr size = sizeof(glm::vec4) * this->numTiles.x * this->numTiles.y * this->numTiles.z * 2;
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, (void*)0, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->clustersSSBOBinding, this->clustersSSBO);
		this->clustersRes = this->numTiles;
		std::cout << "REALLOCATING clustersSSBO\n";
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


		// Load the compute shader if we haven't already
		if (this->clusterGenShader.getID() == 0) {
			this->clusterGenShader.readCompute(
				"shaders/opengl/clustersgen.glsl"
			);
		}
		this->clusterGenShader.bind();
		this->clusterGenShader.setUniform1f("zNear", camera->projectionParams.perspective.near);
		this->clusterGenShader.setUniform1f("zFar", camera->projectionParams.perspective.far);
		glm::mat4 invProj = glm::inverse(camera->getProjectionMatrix());
		this->clusterGenShader.setUniformMat4("inverseProjection", invProj);
		glDispatchCompute((GLuint)this->numTiles.x, (GLuint)this->numTiles.y, (GLuint)this->numTiles.z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
}





void RP_Deferred_OpenGL::runClustersGPU(Scene* scene) {
	// Also runs cluster AABB gen compute shader if needed.
	this->updateClustersSSBO(scene);
	// The next two just make sure the buffers are sufficiently large.
	this->updateLightsIndexSSBO();
	this->updateTileLightMappingSSBO();

	if (this->clusterCullLightsShader.getID() == 0) {
		this->clusterCullLightsShader.readCompute(
			"shaders/opengl/clusterscull2.glsl"
		);
	}
	this->clusterCullLightsShader.bind();
	//glDispatchCompute(1, 1, 1);
	glDispatchCompute((GLuint)this->numTiles.x, (GLuint)this->numTiles.y, (GLuint)this->numTiles.z);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->tileLightMappingSSBO);
	//GLint* buf = (GLint*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//std::cout << "BEGIN GRID BUFFER\n";
	//for (int i = 0; i < this->numTiles.x * this->numTiles.y * this->numTiles.z * 2; i++) {
	//	std::cout << buf[i] << " ";
	//}
	//std::cout << "END GRID BUFFER\n";
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->lightsIndexSSBO);
	//buf = (GLint*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//std::cout << "BEGIN INDEX BUFFER\n";
	//for (int i = 0; i < lightsIndexSSBOSize; i++) {
	//	std::cout << buf[i] << " ";
	//}
	//std::cout << "END INDEX BUFFER\n";
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->globalIndexCountSSBO);
	//GLuint* buf = (GLuint*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//std::cout << "GLOBAL INDEX: " << *buf << "\n";
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

