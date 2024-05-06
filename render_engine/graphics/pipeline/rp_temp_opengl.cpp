#include "graphics/pipeline/rp_temp_opengl.h"
#include "core/scene.h"
#include "objects/gameobject.h"
#include "objects/go_camera.h"

#include "glm/ext/matrix_clip_space.hpp"

#include <iostream>


RP_Temp_OpenGL::RP_Temp_OpenGL(Graphics& graphics) : RP_Temp(graphics) {}

void RP_Temp_OpenGL::init() {
	this->tempShader.read("shaders/opengl/temp.vert", "shaders/opengl/temp.frag");
}


static void bindMaterial(Shader_OpenGL& shader, Ref<Material> material) {
	// TODO: Support binding different types/more complex materials.
	if (material) {
		shader.setUniform4f("colorDiffuse", material->getDiffuseColor());
		if (material->getDiffuseTexture()) {
			GPUTexture* gpuTex = material->getDiffuseTexture()->getGPUTexture();
			shader.setUniformTex("textureDiffuse",
				((GPUTexture_OpenGL*)gpuTex)->getGLTexID(),
				0, GL_TEXTURE_2D
			);
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
	// TODO: Get perspective matrix from active camera.
	glm::mat4 mvpMat = projMat * mvMat;
	shader.setUniformMat4("mvMat", mvMat);
	shader.setUniformMat4("normalMat", glm::inverse(glm::transpose(mvMat)));
	shader.setUniformMat4("mvpMat", mvpMat);
	// TODO: Get camera pos/dir.
	shader.setUniform3f("cameraPos", glm::vec3(0.0f));
	shader.setUniform3f("cameraDir", glm::vec3(0.0f, 0.0f, -1.0f));

	obj->draw();

	for (auto& child : obj->getChildren()) {
		renderSubtree(shader, child.get(), viewMat, projMat);
	}
}

void RP_Temp_OpenGL::render(Scene* scene) {

	glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	this->tempShader.bind();
	this->tempShader.setUniform1f("specularShininess", 6.0f);
	this->tempShader.setUniform1f("specular", 1.0f);

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

	// TODO: Get view matrix from active camera.
	renderSubtree(this->tempShader, scene->getRoot().get(), viewMatrix, projMatrix);

	this->thisGraphics->swapBuffers();
}

void RP_Temp_OpenGL::renderMesh(Mesh* mesh) {
	GPUMesh* gpuMesh = mesh->getGPUMesh();
	if (gpuMesh) {
		if (mesh->getMaterial()) {
			bindMaterial(this->tempShader, mesh->getMaterial());
		}
		gpuMesh->draw();
	}
}
