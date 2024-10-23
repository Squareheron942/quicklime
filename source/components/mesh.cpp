#include "mesh.h"
#include "componentmanager.h"
#include "console.h"
#include "gameobject.h"
#include "ql_assert.h"
#include "stats.h"
#include <memory>

namespace {
	u64 permut_from_num_attr(u8 n) {
		u64 perm = 0;
		for (u8 i = 0; i < n; i++) {
			perm |= ((i & 0xF) << (i << 2));
		} // for each attrib add the corresponding number (for 3 attribs, should
		  // make 0x210 aka 528)
		return perm;
	}
} // namespace

ql::mesh::mesh(void *vertices, const mdlLoader::mdl_header &hdr)
	: numVerts(hdr.numVerts), vertsize(hdr.sv), _vertices(vertices),
	  radius(hdr.radius) {
	ASSERT(vertices != nullptr, "Invalid vertex data");

	// make sure the vertex data is in ram and not CPU cache
	GSPGPU_FlushDataCache(vertices, hdr.sv * hdr.numVerts);

	// Configure attributes for use with the vertex shader
	AttrInfo_Init(&attrInfo);
	for (int i = 0; i < hdr.attrnum; i++)
		AttrInfo_AddLoader(&attrInfo, i, (GPU_FORMATS)hdr.attrtypes[i],
						   hdr.attrlen[i]);

	BufInfo_Init(&buf);
	BufInfo_Add(&buf, vertices, hdr.sv, hdr.attrnum,
				permut_from_num_attr(hdr.attrnum));
	stats::_vertices += numVerts;
}

ql::mesh::~mesh() {
	linearFree(_vertices);
	stats::_vertices -= numVerts;
}