#pragma once
#include "core/renderengine.h"
#include "graphics/texture.h"
#include "objects/gameobject.h"
#include <filesystem>


/*
* Assets
* 
* Assets, in the context of the game engine, refers to any media file
* that is loaded and used in a game. This can include textures, objects,
* sounds, fonts, etc.
* 
* Assets.h/cpp interfaces with respective libraries and modules to import
* (and sometimes export) all kinds of assets.
*/



class Assets {
public:



	static Ref<GameObject> importObject(RenderEngine& engine, std::filesystem::path path);

	static Ref<Texture> importTexture(RenderEngine& engine, std::filesystem::path path);


};
