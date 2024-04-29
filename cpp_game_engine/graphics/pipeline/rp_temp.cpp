#include "graphics/pipeline/rp_temp.h"


RP_Temp::RP_Temp(Graphics& graphics) : RenderPipeline(graphics) {}

RenderPipelineType RP_Temp::getType() {
	return RenderPipelineType::Temp;
}

std::string RP_Temp::getName() {
	return "Temp";
}