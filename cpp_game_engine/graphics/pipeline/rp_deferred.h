#pragma once
#include "graphics/pipeline/renderpipeline.h"

/*
* ===== Deferred =====
* OpenGL (RP_Deferred_OpenGL)
*
* A deferred render pipeline.
*
*/
class RP_Deferred : public RenderPipeline {
public:

	RP_Deferred(Graphics& graphics);

	virtual RenderPipelineType getType() override;
	virtual std::string getName() override;

};