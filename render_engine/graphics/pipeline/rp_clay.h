#pragma once
#include "graphics/pipeline/renderpipeline.h"

/*
* ===== Clay =====
* OpenGL (RP_Clay_OpenGL)
* A simple feed-forward pipeline designed to test geometry rendering.
* No scene light sources; uses view-based lighting.
* 
* 1. Renders all objects to the main framebuffer without sorting.
* 
*/
class RP_Clay : public RenderPipeline {
public:

	RP_Clay(Graphics& graphics);

	/*
	* There is no implementation in this class; see RP_Clay_OpenGL.
	* 
	* TODO: Add a mechanism to modify settings.
	*/

	virtual RenderPipelineType getType() override;
	virtual std::string getName() override;

};