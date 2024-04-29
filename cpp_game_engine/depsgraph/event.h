#pragma once
#include "depsnode.h"

#include "glm/glm.hpp"



class Event : public DepsNode {
public:

	// TODO: make private?
	bool dirty = false;
	

};


/*
* ===== Standard Events =====
*/

namespace StandardEvents {

	/*
	* The user or system has moved the cursor.
	* pos is the absolute position of the cursor, delta is its motion since the last
	* event. Depending on the cursor input mode set of the InputContext, these may be
	* in screen coordinates or a virtual coordinate system.
	*/
	class CursorMove : public Event {
	public:
		glm::vec2 pos;
		glm::vec2 delta;
	};

	/*
	* The primary window framebuffer has been resized.
	* The width and height are the size in pixels of the new framebuffer. This is
	* not necessarily the same as the size of the window in screen coordinates.
	*/
	class ResizeFramebuffer : public Event {
	public:
		size_t width;
		size_t height;
	};

}
