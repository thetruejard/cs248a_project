#include "graphics/pipeline/rp_none_opengl.h"
#include "core/scene.h"
#include "objects/gameobject.h"
#include "objects/go_camera.h"
#include "utils/printutils.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>



RP_None_OpenGL::RP_None_OpenGL(Graphics& graphics) : RP_None(graphics) {}


void RP_None_OpenGL::init() {}

void RP_None_OpenGL::resizeFramebuffer(size_t width, size_t height) {}


void RP_None_OpenGL::render(Scene* scene) {
	this->thisGraphics->swapBuffers();
}

void RP_None_OpenGL::renderMesh(Mesh* mesh) {}

void RP_None_OpenGL::renderPrimitive(
	Rectangle rect, Ref<Material> material
) {}

