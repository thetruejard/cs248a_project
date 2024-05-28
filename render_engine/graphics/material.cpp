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

void Material::assignMetalnessTexture(const Ref<Texture>& texture) {
	this->metalnessTexture = texture;
}
void Material::assignMetalness(float metalness) {
	this->metalness = metalness;
}

void Material::assignRoughnessTexture(const Ref<Texture>& texture) {
	this->roughnessTexture = texture;
}
void Material::assignRoughness(float roughness) {
	this->roughness = roughness;
}

void Material::assignNormalTexture(const Ref<Texture>& texture) {
	this->normalTexture = texture;
}

Ref<Texture> Material::getDiffuseTexture() {
	return this->diffuseTexture;
}
glm::vec4 Material::getDiffuseColor() {
	return this->diffuseColor;
}

Ref<Texture> Material::getMetalnessTexture() {
	return this->metalnessTexture;
}
float Material::getMetalness() {
	return this->metalness;
}

Ref<Texture> Material::getRoughnessTexture() {
	return this->roughnessTexture;
}
float Material::getRoughness() {
	return this->roughness;
}

Ref<Texture> Material::getNormalTexture() {
	return this->normalTexture;
}