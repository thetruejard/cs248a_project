#include "graphics/pipeline/rp_deferred.h"


RP_Deferred::RP_Deferred(Graphics& graphics) : RenderPipeline(graphics) {}

RenderPipelineType RP_Deferred::getType() {
	return RenderPipelineType::Deferred;
}

std::string RP_Deferred::getName() {
	return "Deferred";
}