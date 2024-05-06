#pragma once
#include <cstdint>

class RenderEngine;


/*
* An InputContext instance
*/
class InputContext {
public:

	InputContext(RenderEngine& engine);

	void _charCallback(uint32_t utf32_codepoint);
	void _cursorPosCallback(double xpos, double ypos);
	void _keyCallback(int key, int scancode, int action, int mods);

	// TEMP: Find better way to map/track the input state.
	bool getKeyState(int glfw_keycode);


private:

	RenderEngine* thisEngine;


};