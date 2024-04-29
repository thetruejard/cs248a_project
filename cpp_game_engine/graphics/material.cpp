#include "graphics/material.h"


Material::Material(MaterialID id) : Datablock(id) {}

void Material::setName(std::string name) {
	this->name = name;
}
std::string Material::getName() {
	return this->name;
}

void Material::assignDiffuseTexture(const Ref<Texture>& texture) {
	this->diffuseTexture = texture;
}
void Material::assignDiffuseColor(glm::vec4 diffuseColor) {
	this->diffuseColor = diffuseColor;
}

Ref<Texture> Material::getDiffuseTexture() {
	return this->diffuseTexture;
}
glm::vec4 Material::getDiffuseColor() {
	return this->diffuseColor;
}
