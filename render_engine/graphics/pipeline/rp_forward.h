#pragma once
#pragma once
#include "graphics/pipeline/renderpipeline.h"

/*
* ===== Forward =====
* OpenGL (RP_Forward_OpenGL)
*
* A forwardrender pipeline.
*
*/
class RP_Forward : public RenderPipeline {
public:

	RP_Forward(Graphics& graphics);

	virtual RenderPipelineType getType() override;
	virtual std::string getName() override;

};