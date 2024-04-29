#pragma once
#include <utility>
#include <vector>

struct GLFWwindow;
class CppGameEngine;
class InputContext;


/*
* Callbacks_GLFW takes care of routing callbacks and callback translations.
* 
* Although all GLFW callbacks are routed through this class, most invoked callbacks
* are input events that must be handled quickly, so emphasis is placed on handling
* input callbacks efficiently.
* 
* GLFW delivers input events (keyboard, mouse, joysticks, etc.) through generic
* callbacks specified by function-pointers. This means the only indication of which
* window received the input is the GLFWwindow* pointer delivered through the callback.
* We want to send input events to the InputContext of the relevant CppGameEngine
* instance, so we send the callbacks to Callbacks_GLFW, which finds which instance of
* the engine is being invoked.
* 
* Basically:
* [OS] -> [GLFW] -> Callbacks_GLFW -> InputContext
* 
* Some of the higher-level callbacks are not invoked by all relevant events. For
* example, charCallback is not invoked by the backspace or tab keys.
* Callback translations take lower-level events, such as keypresses, and translate
* them into higher-level events, such as special chars.
* When a translation is made, BOTH the lower-level and higher-level callbacks are
* invoked in an unspecified order.
* 
* The following callback translations are currently supported:
* 
* - keyCallback -> charCallback
*	- backspace, tab
*/
class Callbacks_GLFW {
public:

	/*
	* Registers a mapping between a GLFWwindow and its corresponding InputContext,
	* so that the callbacks know which InputContext to invoke.
	*/
	static void registerWindow(GLFWwindow* wnd, CppGameEngine* engine);
	static void unregisterWindow(GLFWwindow* wnd);

	// glfwSetCharCallback
	static void _charCallback(
		GLFWwindow* wnd, unsigned int codepoint
	);

	// glfwSetCursorPosCallback
	static void _cursorPosCallback(
		GLFWwindow* wnd, double xpos, double ypos
	);

	// glfwSetKeyCallback
	static void _keyCallback(
		GLFWwindow* wnd, int key, int scancode, int action, int mods
	);

	// glfwSetFramebufferSizeCallback
	static void _framebufferSizeCallback(
		GLFWwindow* wnd, int width, int height
	);


private:

	struct Mapping {
		GLFWwindow* wnd;
		CppGameEngine* engine;
		InputContext* inctx;
	};

	/*
	* Because the number of windows is typically very low (almost always 1),
	* it's faster in most cases to store a primary window mapping for fast lookup,
	* and keep any supplementary ones in a vector for quick iteration.
	*/
	static Mapping primary;
	static std::vector<Mapping> mappings;

	/*
	* Looks up a mapping.
	*/
	static CppGameEngine* lookup_engine(GLFWwindow* wnd);
	static InputContext* lookup_inctx(GLFWwindow* wnd);

};