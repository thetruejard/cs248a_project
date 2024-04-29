#include "graphics/texture.h"
#include "core/CppGameEngine.h"


Texture::Texture(TextureID id, CppGameEngine* engine) :
	Datablock(id), thisGraphics(engine->getGraphics()) {}
Texture::~Texture() {
	if (this->gpuTexture) {
		delete this->gpuTexture;
		this->gpuTexture = nullptr;
	}
}

bool Texture::upload(
	uint8_t* data,
	size_t width,
	size_t height,
	size_t numChannels
) {
	if (!this->gpuTexture) {
		this->gpuTexture = thisGraphics->createTexture(this);
	}
	return this->gpuTexture->upload(data, width, height, numChannels);
}


void Texture::setPath(std::filesystem::path path) {
	this->path = path;
}

const std::filesystem::path& Texture::getPath() {
	return this->path;
}

size_t Texture::getWidth() {
	return this->width;
}
size_t Texture::getHeight() {
	return this->height;
}
size_t Texture::getNumChannels() {
	return this->numChannels;
}

GPUTexture* Texture::getGPUTexture() {
	return this->gpuTexture;
}
