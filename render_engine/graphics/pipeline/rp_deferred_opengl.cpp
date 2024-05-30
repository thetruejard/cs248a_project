#include "graphics/pipeline/rp_deferred_opengl.h"
#include "core/scene.h"
#include "objects/gameobject.h"
#include "objects/go_camera.h"
#include "utils/printutils.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>



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
	this->lightShader.setUniform1f("specularShininess", 6.0f);
	this->lightShader.setUniform1f("specular", 0.1f);

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


	this->lightShader.setUniformTex("texturePos", this->gbPosTex, 0);
	this->lightShader.setUniformTex("textureNormals", this->gbNormalTex, 1);
	this->lightShader.setUniformTex("textureAlbedo", this->gbAlbedoTex, 2);
	this->lightShader.setUniformTex("textureMetalRough", this->gbMetalRoughTex, 3);

	// TODO: based on culling method, use SSBO instead of the following
	// updateLightsSSBO(Scene* scene, glm::mat4 viewMatrix)
	

	if (this->culling != LightCulling::RasterSphere) {

		this->updateLightsSSBO(scene, viewMatrix);
		this->lightShader.setUniform2i("cullingMethod", glm::ivec2((GLint)this->culling, 0));
		this->thisGraphics->primitives.rectangle->draw();

	}
	else {
		// Raster sphere

		// TODO: Make this a sphere
		for (GO_Light* light : scene->lights) {
			glm::vec4 posVector = viewMatrix * light->getModelMatrix()[3];
			this->lightShader.setUniform4f("light.positionType",
				glm::vec4(posVector.x, posVector.y, posVector.z, (float)light->type)
			);
			glm::vec4 dirVector = viewMatrix * glm::vec4(light->getWorldSpaceDirection(), 0.0f);
			this->lightShader.setUniform3f("light.direction", glm::normalize(dirVector));
			this->lightShader.setUniform2f("light.innerOuterAngles", light->innerOuterAngles);
			this->lightShader.setUniform3f("light.color", light->color);
			this->lightShader.setUniform3f("light.attenuation", light->attenuation);
			this->thisGraphics->primitives.rectangle->draw();
		}

	}



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
int x = sizeof(SSBOLight);

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
