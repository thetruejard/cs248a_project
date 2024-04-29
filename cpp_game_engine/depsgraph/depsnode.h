#pragma once


/*
* An element in a depsgraph.
* Child nodes are dependent on their parents.
*/
class DepsNode {
public:

	void addChild(DepsNode* child);


};