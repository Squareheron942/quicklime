#ifndef OBJGL_H_
#define OBJGL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * *indices - pointer to the indices with that material assigned
 * len - number of indices with that material
 * *name - pointer to a null terminated material name
 *
 * please note that *indices uses the same memory as *indices in ObjGLData,
 * just offset.
 */
typedef struct{
	unsigned int *indices;
	unsigned int len;
	char *name;
} objglMaterial;

/*
 * *data - pointer to an interleaved vertex data
 * *indices - pointer to indices (for glDrawElements)
 * *materials - pointer to materials
 * numIndices - total number of indices
 * numVertices - total number of unique indices
 * vertSize - bytes per one vertex (12, 20, 24 or 32)
 * numMaterials - number of materials
 * hasNormals (boolean) - whether or not the normals were present in an OBJ and read
 * hasTexCoords (boolean) - whether or not the UV coords were present in an OBJ and read
 */
typedef struct{
	float *data;
	unsigned int *indices;
	objglMaterial *materials;
	unsigned int numIndices, numVertices, vertSize, numMaterials;
	unsigned char hasNormals, hasTexCoords;
} ObjGLData;

/*
 * in *buffer - OBJ file contents as a C-string
 * returns the OBJData
 */
ObjGLData objgl_loadObj(const char* buffer);

/*
 * frees and zeros out the given OBJData
 */
void objgl_delete(ObjGLData* data);

#ifdef __cplusplus
}
#endif

#endif
