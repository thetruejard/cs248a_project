#include "assets/assets.h"

#include "stb/stb_image.h"

#include <iostream>


Ref<Texture> Assets::importTexture(RenderEngine& engine, std::filesystem::path path) {
	Ref<Texture> texture = engine.getTextureByPath(path);
	if (texture) {
		return texture;
	}

	// Try loading before creating a new texture, in case it fails.
	int width, height, numChannels;
	stbi_set_flip_vertically_on_load(1);
	uint8_t* data = stbi_load(path.string().c_str(), &width, &height, &numChannels, 0);
	if (!data || width <= 0 || height <= 0 && numChannels < 1 || numChannels > 4) {
		// TODO: Error handling.
		std::cout << "Failed to load texture: " << path << "\n";
		std::cout << "data: " << (size_t)data << " | width: " << width << " | height: " << height
			<< " | numChannels: " << numChannels << "\n";
		std::cout << "STBI reason: " << stbi_failure_reason() << "\n";
		return Ref<Texture>();
	}
	
	texture = engine.createTexture();
	if (!texture) {
		std::cout << "Failed to create new texture??\n";
	}
	else {
		texture->upload(data, (size_t)width, (size_t)height, (size_t)numChannels);
	}

	stbi_image_free(data);
	return texture;
}