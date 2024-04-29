#include "core/cppgameengine.h"
#include "graphics/graphics.h"
#include "io/callbacks_glfw.h"

#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <atomic>
#include <chrono>
#include <iostream>


// TODO: TEMP until a logging system is made.
static void glfwError(int error_code, const char* desc) {
	std::cerr << "GLFW error [" << error_code << "]: " << desc << "\n";
}


CppGameEngine::CppGameEngine() : CppGameEngine(Graphics::Backend::NONE) {}

CppGameEngine::CppGameEngine(Graphics::Backend backend) : inputContext(*this) {
	static bool glfwInitialized = false;
	if (!glfwInitialized) {
		glfwSetErrorCallback(glfwError);
		if (!glfwInit()) {
			return;
		}
		glfwInitialized = true;
	}
	this->graphics = Graphics::openGraphics(this, backend);
	// TODO: Error handling
	if (this->graphics) {
		this->graphics->initPrimitives();
	}
}

CppGameEngine::~CppGameEngine() {
	if (this->graphics != nullptr) {
		Callbacks_GLFW::unregisterWindow(this->graphics->getWindow());
		this->graphics->destroyWindow();
		delete this->graphics;
		this->graphics = nullptr;
	}
}


void CppGameEngine::launch(
	std::string windowTitle,
	size_t width,
	size_t height,
	bool fullscreen
) {
	if (this->graphics == nullptr) {
		return;
	}

	this->windowTitle = windowTitle;
	this->graphics->createWindow(this->windowTitle, width, height, fullscreen);
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(this->graphics->getWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	Callbacks_GLFW::registerWindow(this->graphics->getWindow(), this);

	auto lasttime = std::chrono::high_resolution_clock::now();

	bool done = false;
	while (!done) {
		auto now = std::chrono::high_resolution_clock::now();
		auto diff = now - lasttime;
		lasttime = now;
		float deltaTime = diff.count() / 1000000000.0f;

		this->depsgraph.resolveGraph();

		if (this->activeScene) {
			this->activeScene->evaluateComponents(deltaTime);
		}

		// TODO (in the long run): Consider double buffering this data, if feasible.
		this->graphics->render(this->activeScene.get());
		done = !this->graphics->pollEvents() || done;
	}

	Callbacks_GLFW::unregisterWindow(this->graphics->getWindow());
	this->graphics->destroyWindow();

}


Graphics* CppGameEngine::getGraphics() {
	return this->graphics;
}

InputContext* CppGameEngine::getInputContext() {
	return &this->inputContext;
}

Depsgraph* CppGameEngine::getDepsgraph() {
	return &this->depsgraph;
}

std::string CppGameEngine::getWindowTitle() {
	return this->windowTitle;
}

void CppGameEngine::setWindowTitle(std::string title) {
	this->windowTitle = title;
	if (this->graphics && this->graphics->getWindow()) {
		glfwSetWindowTitle(this->graphics->getWindow(), title.c_str());
	}
}


Ref<Scene> CppGameEngine::createScene() {
	return this->scenes.create(this);
}

void CppGameEngine::setActiveScene(Ref<Scene> scene) {
	this->activeScene = scene;
}

Ref<Scene> CppGameEngine::getActiveScene() {
	return this->activeScene;
}



Ref<GameObject> CppGameEngine::getObjectByID(GameObjectID id) {
	return this->objects.getByID(id);
}


Ref<Mesh> CppGameEngine::createMesh() {
	return this->meshes.create(this);
}

Ref<Material> CppGameEngine::createMaterial() {
	return this->materials.create();
}

Ref<Texture> CppGameEngine::createTexture() {
	return this->textures.create(this);
}


Ref<Texture> CppGameEngine::getTextureByPath(const std::filesystem::path& path) {
	for (const Ref<Texture>& texture : this->textures.iterate()) {
		if (!texture->getPath().empty() &&
			std::filesystem::equivalent(texture->getPath(), path)) {
			return texture;
		}
	}
	return Ref<Texture>();
}
