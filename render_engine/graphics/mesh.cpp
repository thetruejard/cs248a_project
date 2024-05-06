#include "graphics/mesh.h"
#include "core/renderengine.h"


Mesh::Mesh(MeshID id, RenderEngine* engine) :
	Datablock(id), thisGraphics(engine->getGraphics()) {}
Mesh::~Mesh() {
	if (this->gpuMesh) {
		delete this->gpuMesh;
		this->gpuMesh = nullptr;
	}
}

Vertex* Mesh::createVertexBuffer(size_t numVertices) {
	this->vertices.resize(numVertices);
	return vertices.data();
}
VertexIndex* Mesh::createIndexBuffer(size_t numIndices) {
	this->indices.resize(numIndices);
	return indices.data();
}

const std::vector<Vertex>& Mesh::getVertices() const {
	return this->vertices;
}
const std::vector<VertexIndex>& Mesh::getIndices() const {
	return this->indices;
}

void Mesh::uploadMesh() {
	if (!this->thisGraphics) {
		return;
	}
	if (this->gpuMesh) {
		delete this->gpuMesh;
	}
	this->gpuMesh = this->thisGraphics->createMesh();
	this->gpuMesh->uploadFrom(*this);
}

void Mesh::assignMaterial(const Ref<Material>& material) {
	this->material = material;
}

Ref<Material> Mesh::getMaterial() {
	return this->material;
}

GPUMesh* Mesh::getGPUMesh() {
	return this->gpuMesh;
}

void Mesh::draw() {
	this->thisGraphics->renderMesh(this);
}
