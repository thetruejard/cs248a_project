#include "io/inputcontext.h"
#include "core/cppgameengine.h"

#include "GLFW/glfw3.h"

#include <iostream>


InputContext::InputContext(CppGameEngine& engine) : thisEngine(&engine) {}


void InputContext::_charCallback(uint32_t utf32_codepoint) {
	// TODO
	return;

	std::string t = this->thisEngine->getWindowTitle();
	if ((char)utf32_codepoint == '\b') {
		if (t.length() > 0) {
			t.pop_back();
		}
	}
	else {
		t += (char)utf32_codepoint;
	}
	this->thisEngine->setWindowTitle(t);
}

void InputContext::_cursorPosCallback(double xpos, double ypos) {
	this->thisEngine->getDepsgraph()->_invokeCursorMove(
		(float)xpos, (float)ypos
	);
}

void InputContext::_keyCallback(int key, int scancode, int action, int mods) {
	// TODO
	const char* kc = glfwGetKeyName(key, scancode);
	std::string k = kc ? kc : "[unknown]";
	std::string actstr;
	switch (action) {
	case GLFW_PRESS: actstr = "pressed"; break;
	case GLFW_RELEASE: actstr = "released"; break;
	case GLFW_REPEAT: actstr = "repeated"; break;
	}
	//std::cout << "Key " << actstr << ": " << k << "\n";

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		GLFWwindow* wnd = this->thisEngine->getGraphics()->getWindow();
		int mode = glfwGetInputMode(wnd, GLFW_CURSOR);
		if (mode == GLFW_CURSOR_DISABLED) {
			glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else {
			glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
}


bool InputContext::getKeyState(int glfw_keycode) {
	if (thisEngine->getGraphics()) {
		return glfwGetKey(thisEngine->getGraphics()->getWindow(), glfw_keycode) == GLFW_PRESS;
	}
	return false;
}