#pragma once
#include "core/Datablock.h"

#include <cstdint>
#include <filesystem>


class RenderEngine;
class GPUTexture;
class Graphics;

DATABLOCK_ID(Texture);


/*
* Represents a single texture.
* For now, all textures are 8bpp and 1-4 channels.
* 
* TODO: Revise to support different formats:
*	- sRGB color space
*	- float textures
*/
class Texture : public Datablock {
public:

	Texture(TextureID id, RenderEngine* engine);
	~Texture();

	bool upload(
		uint8_t* data,
		size_t width,
		size_t height,
		size_t numChannels
	);

	void setPath(std::filesystem::path path);
	const std::filesystem::path& getPath();

	size_t getWidth();
	size_t getHeight();
	size_t getNumChannels();

	GPUTexture* getGPUTexture();


private:

	Graphics* thisGraphics;
	GPUTexture* gpuTexture = nullptr;

	std::filesystem::path path;

	size_t width = 0;
	size_t height = 0;
	size_t numChannels = 0;

};
