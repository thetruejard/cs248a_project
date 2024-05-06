#pragma once
#include "depsgraph/event.h"

#include <functional>
#include <vector>


/*
* "Depsgraph" is short for "Dependency Graph."
* 
* A depsgraph is a DAG that maps relationships between elements in the form of
* dependencies. For example, the orientation of a POV camera is dependent on
* mouse motion, so their respective nodes would be connected by an edge.
* 
* TODO: The depsgraph is in its elementary stages, and currently only supports
* hooking callbacks into a few pre-defined "standard" events.
*/
class Depsgraph {
public:

	void _invokeCursorMove(float x, float y);
	void _invokeFramebufferResize(size_t width, size_t height);

	void hookEvent(std::function<void(const StandardEvents::CursorMove*)> f);
	void hookEvent(std::function<void(const StandardEvents::ResizeFramebuffer*)> f);

	void resolveGraph();


private:

	StandardEvents::CursorMove cursorMove;
	StandardEvents::ResizeFramebuffer resizeFramebuffer;

	std::vector<std::function<void(const StandardEvents::CursorMove*)>> cursorMoveEvents;
	std::vector<std::function<void(const StandardEvents::ResizeFramebuffer*)>> resizeFramebufferEvents;

};