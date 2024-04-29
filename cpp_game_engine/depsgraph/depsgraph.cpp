#include "depsgraph.h"

void Depsgraph::_invokeCursorMove(float x, float y) {
	this->cursorMove.delta.x = x - this->cursorMove.pos.x;
	this->cursorMove.delta.y = y - this->cursorMove.pos.y;
	this->cursorMove.pos.x = x;
	this->cursorMove.pos.y = y;
	this->cursorMove.dirty = true;
}
void Depsgraph::_invokeFramebufferResize(size_t width, size_t height) {
	this->resizeFramebuffer.width = width;
	this->resizeFramebuffer.height = height;
	this->resizeFramebuffer.dirty = true;
}

void Depsgraph::hookEvent(std::function<void(const StandardEvents::CursorMove*)> f) {
	this->cursorMoveEvents.push_back(f);
}
void Depsgraph::hookEvent(std::function<void(const StandardEvents::ResizeFramebuffer*)> f) {
	this->resizeFramebufferEvents.push_back(f);
}

void Depsgraph::resolveGraph() {
	if (this->cursorMove.dirty) {
		for (auto& f : cursorMoveEvents) {
			f(&this->cursorMove);
		}
		this->cursorMove.dirty = false;
	}
	if (this->resizeFramebuffer.dirty) {
		for (auto& f : resizeFramebufferEvents) {
			f(&this->resizeFramebuffer);
		}
		this->resizeFramebuffer.dirty = false;
	}
}