#include "graphics/graphics.h"
#include "core/renderengine.h"
#include "core/scene.h"
#include "graphics/graphics_opengl.h"

#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <iostream>


Graphics::~Graphics() {
	if (this->pipeline) {
		delete this->pipeline;
		this->pipeline = nullptr;
	}
}


Graphics::Backend Graphics::getPreferredBackend() {
	// Create a temporary window to check OpenGL version.
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	GLFWwindow* wnd = glfwCreateWindow(100, 100, "None", NULL, NULL);
	glfwMakeContextCurrent(wnd);
	GLint glVersion;
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion);
	glfwDestroyWindow(wnd);
	if (glVersion >= 4) {
		return Graphics::Backend::OPENGL;
	}
	return Graphics::Backend::NONE;
}

Graphics* Graphics::openGraphics(RenderEngine* engine, Graphics::Backend backend) {
	if (backend == Graphics::Backend::NONE) {
		backend = getPreferredBackend();
	}
	Graphics* r = nullptr;
	switch (backend) {
	case Graphics::Backend::OPENGL:
	{
		Graphics_OpenGL* g = new Graphics_OpenGL();
		if (g->getWindow()) {
			r = (Graphics*)g;
		}
		else {
			delete g;
			r = nullptr;
		}
		break;
	}
	default:
		r = nullptr;
		break;
	}

	if (r) {
		r->thisEngine = engine;
	}
	return r;
}

Graphics::Backend Graphics::getBackend() {
	return this->backend;
}

std::string Graphics::getBackendString() {
	return "None";
}

std::string Graphics::getGPUNameString() {
	return "None";
}

GLFWwindow* Graphics::getWindow() {
	return this->window;
}

// TODO: Cache?
size_t Graphics::getWidth() {
	int w, h;
	glfwGetFramebufferSize(this->window, &w, &h);
	return (size_t)w;
}

size_t Graphics::getHeight() {
	int w, h;
	glfwGetFramebufferSize(this->window, &w, &h);
	return (size_t)h;
}

void Graphics::render(Scene* scene) {
	if (this->pipeline) {
		this->pipeline->render(scene);
	}
}

void Graphics::renderMesh(Mesh* mesh) {
	if (this->pipeline) {
		this->pipeline->renderMesh(mesh);
	}
}
void Graphics::renderPrimitive(Rectangle rect, Ref<Material> material) {
	if (this->pipeline) {
		this->pipeline->renderPrimitive(rect, material);
	}
}


void Graphics::initPrimitives() {
	Ref<Mesh> mesh;

	// Rectangle.
	this->primitives.rectangle = this->thisEngine->createMesh();
	Rectangle rect;
	rect.bottomLeft = glm::vec2(0.0f);
	rect.widthHeight = glm::vec2(1.0f);
	if (!rect.toMesh(this->primitives.rectangle)) {
		std::cout << "Failed to initialize primitive \"Rectangle\"\n";
	}
	this->primitives.rectangle->uploadMesh();
}



GPUTexture::GPUTexture(Texture* thisTexture) {
	this->thisTexture = thisTexture;
}
