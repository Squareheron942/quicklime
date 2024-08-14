#pragma once

#include "util/slmdlloader.h"
#include <citro3d.h>
#include <memory>
#include <vector>

class GameObject;

struct LOD_info {
	float distance2; // LOD distance^2
	int beginindex, size;
};

class mesh {
	friend class MeshRenderer;
	// Configure attributes for use with the vertex shader
	C3D_AttrInfo attrInfo;
	C3D_BufInfo buf;
	unsigned int numVerts;
	unsigned int vertsize;
	void *_vertices;
	float radius;
	std::vector<LOD_info> LOD_levels;

  public:
	const void *vertices() const { return _vertices; }
	mesh(void *vertices, const mdlLoader::mdl_header &hdr);
	~mesh();
};