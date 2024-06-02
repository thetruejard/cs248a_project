#include "graphics/pipeline/rp_none.h"


RP_None::RP_None(Graphics& graphics) : RenderPipeline(graphics) {}

RenderPipelineType RP_None::getType() {
	return RenderPipelineType::None;
}

std::string RP_None::getName() {
	return "None";
}