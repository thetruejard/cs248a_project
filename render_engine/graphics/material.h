#pragma once
#include "core/datablock.h"
#include "graphics/texture.h"

#include "glm/glm.hpp"


DATABLOCK_ID(Material);

/*
* Materials store info about how a given mesh will be drawn.
* Materials are universal to all graphics backends and render pipelines.
* 
* TODO: Consider supporting multiple of each type of texture.
*/
class Material : public Datablock {
public:

	Material(MaterialID id);

	void setName(std::string name);
	std::string getName();

	void assignDiffuseTexture(const Ref<Texture>& texture);
	void assignDiffuseColor(glm::vec4 diffuseColor);

	Ref<Texture> getDiffuseTexture();
	glm::vec4 getDiffuseColor();


	// TODO: TEMP
	bool wireframe = false;


private:

	std::string name;

	Ref<Texture> diffuseTexture;
	glm::vec4 diffuseColor;

};