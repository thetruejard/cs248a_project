#include "io/callbacks_glfw.h"
#include "core/cppgameengine.h"

#include "GLFW/glfw3.h"


void Callbacks_GLFW::registerWindow(GLFWwindow* wnd, CppGameEngine* engine) {
	CppGameEngine* existing = lookup_engine(wnd);
	if (existing) {
		// TODO: Warning/error? Check whether matches engine.
		return;
	}
	if (!primary.wnd) {
		primary.wnd = wnd;
		primary.engine = engine;
		primary.inctx = engine->getInputContext();
	}
	else {
		Mapping m;
		m.wnd = wnd;
		m.engine = engine;
		m.inctx = engine->getInputContext();;
		mappings.push_back(m);
	}

	glfwSetCharCallback(wnd, _charCallback);
	glfwSetCursorPosCallback(wnd, _cursorPosCallback);
	glfwSetKeyCallback(wnd, _keyCallback);
	glfwSetFramebufferSizeCallback(wnd, _framebufferSizeCallback);
}

void Callbacks_GLFW::unregisterWindow(GLFWwindow* wnd) {
	if (wnd == primary.wnd) {
		if (mappings.empty()) {
			primary = { nullptr, nullptr };
		}
		else {
			primary = mappings.front();
			mappings.erase(mappings.begin());
		}
	}
	else {
		for (size_t i = 0; i < mappings.size(); i++) {
			if (wnd == mappings[i].wnd) {
				mappings.erase(mappings.begin() + i);
				return;
			}
		}
	}
}


void Callbacks_GLFW::_charCallback(
	GLFWwindow* wnd, unsigned int codepoint
) {
	InputContext* inctx = lookup_inctx(wnd);
	if (inctx) {
		inctx->_charCallback((uint32_t)codepoint);
	}
}

void Callbacks_GLFW::_cursorPosCallback(
	GLFWwindow* wnd, double xpos, double ypos
) {
	InputContext* inctx = lookup_inctx(wnd);
	if (inctx) {
		inctx->_cursorPosCallback(xpos, ypos);
	}
}

void Callbacks_GLFW::_keyCallback(
	GLFWwindow* wnd, int key, int scancode, int action, int mods
) {
	InputContext* inctx = lookup_inctx(wnd);
	if (inctx) {
		inctx->_keyCallback(key, scancode, action, mods);

		// Translations.
		if (action != GLFW_RELEASE) {
			switch (key) {
			case GLFW_KEY_BACKSPACE:
				inctx->_charCallback((uint32_t)'\b');
				break;
			case GLFW_KEY_TAB:
				inctx->_charCallback((uint32_t)'\t');
				break;
			default:
				break;
			}
		}
	}
}

void Callbacks_GLFW::_framebufferSizeCallback(
	GLFWwindow* wnd, int width, int height
) {
	CppGameEngine* engine = lookup_engine(wnd);
	if (engine) {
		Graphics* graphics = engine->getGraphics();
		if (graphics) {
			graphics->resizeFramebuffer((size_t)width, (size_t)height);
		}
		Depsgraph* dg = engine->getDepsgraph();
		dg->_invokeFramebufferResize((size_t)width, (size_t)height);
	}
}


Callbacks_GLFW::Mapping Callbacks_GLFW::primary = { nullptr, nullptr };
std::vector<Callbacks_GLFW::Mapping> Callbacks_GLFW::mappings;


CppGameEngine* Callbacks_GLFW::lookup_engine(GLFWwindow* wnd) {
	if (wnd == primary.wnd) {
		return primary.engine;
	}
	for (const auto& m : mappings) {
		if (wnd == m.wnd) {
			return m.engine;
		}
	}
	return nullptr;
}

InputContext* Callbacks_GLFW::lookup_inctx(GLFWwindow* wnd) {
	if (wnd == primary.wnd) {
		return primary.inctx;
	}
	for (const auto& m : mappings) {
		if (wnd == m.wnd) {
			return m.inctx;
		}
	}
	return nullptr;
}
