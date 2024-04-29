#include "graphics/pipeline/renderpipeline.h"
#include "graphics/graphics.h"

RenderPipeline::RenderPipeline(Graphics& graphics) : thisGraphics(&graphics) {}

void RenderPipeline::init() {}

void RenderPipeline::resizeFramebuffer(size_t width, size_t height) {}

void RenderPipeline::renderPrimitive(Rectangle rect, Ref<Material> material) {}
