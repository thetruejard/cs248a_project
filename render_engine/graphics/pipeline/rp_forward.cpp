#include "graphics/pipeline/rp_forward.h"


RP_Forward::RP_Forward(Graphics& graphics) : RenderPipeline(graphics) {}

RenderPipelineType RP_Forward::getType() {
	return RenderPipelineType::Forward;
}

std::string RP_Forward::getName() {
	return "Forward";
}