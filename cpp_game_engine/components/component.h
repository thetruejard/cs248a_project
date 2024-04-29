#pragma once
#include <typeinfo>

class GameObject;


/*
* Components are features of a GameObject that define how the object behaves.
* 
* 
* TODO: Components can specify which base type(s) of GameObjects it supports by returning
* their type enum(s) GameObjectType from supportedTypes(). This is enforced by the
* process that adds components to GameObjects, and a Component can safely assume that
* its associated object is one of the type(s) it supports.
* 
* 
* The engine implements Components with the following guarantees:
* 
* 1. evaluate(deltaTime) is called once every frame, where deltaTime is approximately
* the amount of time (in seconds) since the last call.
*	- deltaTime is an approximation and the same value is used for every Component for
*	the entire frame. This is sufficient for most purposes. If a Component needs a
*	more accurate frame time, it will need to maintain its own timer.
*	- "Evaluating" a Component means calling this function.
* 
* 2. Components are ordered on a per-object basis; that is, each object has an ordered
* list of Components, and Components are evaluated in order.
*	- The next Component will not start evaluating until the evaluate() function of
*	the one before it has returned.
* 
* 3. An object's Components will not start evaluating until the last Component of its
* parent object has finished evaluating.
*	- In the big picture, this means an object's Components won't evaluate until all
*	the Components of all its ancestors have been evaluated.
* 
* 
* Components must fulfill the following additional requirements:
*	1. The first argument to all constructors is a pointer to the GameObject this
*	Component is assigned to. Copy- and move-constructors are excepted.
* 
*/
class Component {
public:

	Component(GameObject* object);
	virtual ~Component() = default;

	// Checks whether this Component is of Type and if so, returns a pointer to it.
	// Compares EXACT types; returns nullptr on mismatch even if this Component's type
	// is derived from Type.
	template<typename Type>
	Type* isType() {
		if (typeid(*this) == typeid(Type)) {
			return (Type*)this;
		}
		return nullptr;
	}

	virtual void evaluate(float deltaTime);

	// virtual GameObjectType supportedTypes();

};
