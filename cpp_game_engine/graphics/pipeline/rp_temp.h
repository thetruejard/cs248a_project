#pragma once
#include "graphics/pipeline/renderpipeline.h"

/*
* ===== Temp =====
* OpenGL (RP_Temp_OpenGL)
* 
* A temporary sandbox to test experimental features.
*
*/
class RP_Temp : public RenderPipeline {
public:

	RP_Temp(Graphics& graphics);

	virtual RenderPipelineType getType() override;
	virtual std::string getName() override;

};