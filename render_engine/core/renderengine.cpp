#include "core/renderengine.h"
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


RenderEngine::RenderEngine() : RenderEngine(Graphics::Backend::NONE) {}

RenderEngine::RenderEngine(Graphics::Backend backend) : inputContext(*this) {
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

RenderEngine::~RenderEngine() {
	if (this->graphics != nullptr) {
		Callbacks_GLFW::unregisterWindow(this->graphics->getWindow());
		this->graphics->destroyWindow();
		delete this->graphics;
		this->graphics = nullptr;
	}
}


void RenderEngine::launch(
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


json RenderEngine::launch_eval(
	std::string windowTitle,
	size_t width,
	size_t height,
	bool fullscreen,
	glm::mat4* camMats,
	size_t numCamMats,
	bool log
) {

	if (this->graphics == nullptr) {
		return json();
	}

	this->windowTitle = windowTitle;
	this->graphics->createWindow(this->windowTitle, width, height, fullscreen);
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(this->graphics->getWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	Callbacks_GLFW::registerWindow(this->graphics->getWindow(), this);
	if (this->activeScene && this->activeScene->getActiveCamera()) {
		this->activeScene->getActiveCamera()->setAspect(
			this->graphics->getWidth() / (float)this->graphics->getHeight()
		);
	}

	std::vector<float> loggedFrametimes;
	if (log) {
		loggedFrametimes.reserve(numCamMats+1);
	}


	auto lasttime = std::chrono::high_resolution_clock::now();

	bool done = false;
	for (size_t viewIdx = 0; !done && viewIdx < numCamMats; viewIdx++) {
		auto now = std::chrono::high_resolution_clock::now();
		auto diff = now - lasttime;
		lasttime = now;
		double deltaTime = diff.count() / 1000000000.0;
		if (log) {
			loggedFrametimes.push_back((float)deltaTime);
		}

		this->depsgraph.resolveGraph();
		if (this->activeScene) {
			this->activeScene->evaluateComponents((float)deltaTime);

			if (GO_Camera* cam = this->activeScene->getActiveCamera().get()) {
				cam->setLocalMatrix(camMats[viewIdx]);
			}
		}

		// TODO (in the long run): Consider double buffering this data, if feasible.
		this->graphics->render(this->activeScene.get());
		done = !this->graphics->pollEvents() || done;
	}

	Callbacks_GLFW::unregisterWindow(this->graphics->getWindow());
	this->graphics->destroyWindow();

	if (log) {
		return json(loggedFrametimes);
	}
	return json();

}


Graphics* RenderEngine::getGraphics() {
	return this->graphics;
}

InputContext* RenderEngine::getInputContext() {
	return &this->inputContext;
}

Depsgraph* RenderEngine::getDepsgraph() {
	return &this->depsgraph;
}

std::string RenderEngine::getWindowTitle() {
	return this->windowTitle;
}

void RenderEngine::setWindowTitle(std::string title) {
	this->windowTitle = title;
	if (this->graphics && this->graphics->getWindow()) {
		glfwSetWindowTitle(this->graphics->getWindow(), title.c_str());
	}
}


Ref<Scene> RenderEngine::createScene() {
	return this->scenes.create(this);
}

void RenderEngine::setActiveScene(Ref<Scene> scene) {
	this->activeScene = scene;
}

Ref<Scene> RenderEngine::getActiveScene() {
	return this->activeScene;
}



Ref<GameObject> RenderEngine::getObjectByID(GameObjectID id) {
	return this->objects.getByID(id);
}


Ref<Mesh> RenderEngine::createMesh() {
	return this->meshes.create(this);
}

Ref<Material> RenderEngine::createMaterial() {
	return this->materials.create();
}

Ref<Texture> RenderEngine::createTexture() {
	return this->textures.create(this);
}


Ref<Texture> RenderEngine::getTextureByPath(const std::filesystem::path& path) {
	for (const Ref<Texture>& texture : this->textures.iterate()) {
		if (!texture->getPath().empty() &&
			std::filesystem::equivalent(texture->getPath(), path)) {
			return texture;
		}
	}
	return Ref<Texture>();
}
