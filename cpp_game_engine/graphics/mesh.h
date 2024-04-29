#pragma once
#include "core/Datablock.h"
#include "graphics/material.h"
#include "graphics/vertex.h"

#include <vector>


class CppGameEngine;
class GPUMesh;
class Graphics;

DATABLOCK_ID(Mesh);


/*
* Class representing a mesh.
*/

class Mesh : public Datablock {
public:

	Mesh(MeshID id, CppGameEngine* engine);
	~Mesh();

	Vertex* createVertexBuffer(size_t numVertices);
	VertexIndex* createIndexBuffer(size_t numIndices);
	
	const std::vector<Vertex>& getVertices() const;
	const std::vector<VertexIndex>& getIndices() const;
	
	void uploadMesh();

	void assignMaterial(const Ref<Material>& material);
	Ref<Material> getMaterial();

	GPUMesh* getGPUMesh();

	void draw();

private:

	Graphics* thisGraphics;
	GPUMesh* gpuMesh = nullptr;

	// TODO: For now, vertices are always stored internally.
	// Someday, consider optionally copying directly into a GPU buffer.
	std::vector<Vertex> vertices;
	std::vector<VertexIndex> indices;

	Ref<Material> material;

};