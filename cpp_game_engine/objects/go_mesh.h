#pragma once
#include "graphics/mesh.h"
#include "objects/gameobject.h"


/*
* A simple GameObject containing only a static mesh.
*/
class GO_Mesh : public GameObject {
public:

	GO_Mesh(GameObjectID id, CppGameEngine* engine);
	virtual ~GO_Mesh() override = default;
	virtual std::string getTypeName() override;

	virtual void draw() override;

	void assignMesh(const Ref<Mesh>& mesh);

private:

	Ref<Mesh> mesh;

};