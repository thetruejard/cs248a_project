#include "core/datablock.h"

#include <iostream>


Datablock::Datablock(DatablockID id) : datablockID(id) {}

DatablockID Datablock::getID() {
	return this->datablockID;
}

Ref<Datablock> Datablock::getRef() {
	Ref<Datablock> r = this->datablockSelf.elevate();
	if (!r) {
		// Calling on a dead datablock. This should never happen.
		// TODO: Revise when error handling is improved.
		std::cout << "Fatal error: Called getRef() on a dead Datablock.";
		throw 0;
	}
	return r;
}
WeakRef<Datablock> Datablock::getWeakRef() {
	return this->datablockSelf;
}