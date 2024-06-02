#pragma once
#include "graphics/pipeline/renderpipeline.h"

/*
* ===== None =====
* OpenGL (RP_None_OpenGL)
*
* No render pipeline.
*
*/
class RP_None: public RenderPipeline {
public:

	RP_None(Graphics& graphics);

	virtual RenderPipelineType getType() override;
	virtual std::string getName() override;

};