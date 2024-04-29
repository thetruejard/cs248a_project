#include "graphics/pipeline/rp_clay_opengl.h"
#include "core/scene.h"
#include "objects/gameobject.h"
#include "objects/go_camera.h"

#include "glm/ext/matrix_clip_space.hpp"

#include <iostream>


RP_Clay_OpenGL::RP_Clay_OpenGL(Graphics& graphics) : RP_Clay(graphics) {}

void RP_Clay_OpenGL::init() {
	this->clayShader.read("shaders/opengl/clay.vert", "shaders/opengl/clay.frag");
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
	shader.setUniform3f("clayColor", glm::vec3(1.0f, 0.4f, 0.2f));
	shader.setUniform1f("claySpecularShininess", 6.0f);
	shader.setUniform1f("claySpecular", 1.0f);

	obj->draw();
	for (const Ref<GameObject>& child : obj->getChildren()) {
		renderSubtree(shader, child.get(), viewMat, projMat);
	}
}

void RP_Clay_OpenGL::render(Scene* scene) {

	glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	this->clayShader.bind();

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
	renderSubtree(this->clayShader, scene->getRoot().get(), viewMatrix, projMatrix);

	this->thisGraphics->swapBuffers();
}

void RP_Clay_OpenGL::renderMesh(Mesh* mesh) {
	GPUMesh* gpuMesh = mesh->getGPUMesh();
	if (gpuMesh) {
		gpuMesh->draw();
	}
}
