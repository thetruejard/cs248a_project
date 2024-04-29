#include "graphics/pipeline/rp_clay.h"


RP_Clay::RP_Clay(Graphics& graphics) : RenderPipeline(graphics) {}

RenderPipelineType RP_Clay::getType() {
	return RenderPipelineType::Clay;
}

std::string RP_Clay::getName() {
	return "Clay";
}