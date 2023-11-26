#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <tex3ds.h>
#include <string.h>
#include "fragvshader_shbin.h"
#include "scenemanager.h"
#include "scene1.h"
#include "fast_obj.h"
#include "transform.h"
#include <entt.hpp>
#include "controls.h"
#include "fast_obj.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "objgl.h"
#include "mesh.h"
#include "material.h"
#include "slmdlloader.h"
#include "console.h"
#include "sl_time.h"
#include "componentmanager.h"
#include "script1.h"
#include "movement_script.h"
#include "stats.h"
#include "camera.h"

#define vertex_list_count (sizeof(vertex_list)/sizeof(vertex_list[0]))

#define M_RAD 0.01745329252f

static unsigned int *numvertices = nullptr, numgroups = 0;

// Helper function for loading a texture from memory
bool loadTextureFromMem(C3D_Tex* tex, C3D_TexCube* cube, const void* data, size_t size)
{
	Tex3DS_Texture t3x = Tex3DS_TextureImport(data, size, tex, cube, false);
	if (!t3x){
		printf("didn't find it");
		return false;
	}

	// Delete the t3x object since we don't need it
	Tex3DS_TextureFree(t3x);
	return true;
}

bool loadTextureFromFile(C3D_Tex* tex, C3D_TexCube* cube, const char* path, bool vram) {
        FILE* f = fopen(path, "rb");
		if (!f) {printf("couldn't open file");fclose(f);return false;}
		setvbuf(f, NULL, _IOFBF, 64*1024);
        Tex3DS_Texture t3x = Tex3DS_TextureImportStdio(f, tex, cube, vram);
        fclose(f);
        if (!t3x){
			printf("didn't load it");
            return false;}

        // Delete the t3x object since we don't need it
        Tex3DS_TextureFree(t3x);
		return true;
    }

Scene1::Scene1() : Scene("Scene 1"), script1object(objects) {
	
	script1object.name = "script1object";
	root.addChild(script1object);

	// add components and scripts (components before scripts)
	transform defaultpos({1, 0, 4, 0});
	ComponentManager::addComponent("transform", script1object, &defaultpos);
	ComponentManager::addComponent("mesh", script1object);

	ComponentManager::addScript("MovementScript", script1object);
	ComponentManager::addScript("Script1", script1object);

	r_act_on_objects(&root, &GameObject::Awake); // call awake() on every gameobject and enable them (to self disable do it when this is called)
	
	mesh = fast_obj_read("romfs:/plaza.obj");

	if (mesh->face_count != 0) {
		numgroups = mesh->group_count;
		numvertices = new unsigned int[mesh->group_count];
		meshes = new vertex*[numgroups];
		for (unsigned int ii = 0; ii < mesh->group_count; ii++) 
		{
			const fastObjGroup& grp = mesh->groups[ii];
			numvertices[ii] = grp.face_count * 3;
			meshes[ii] = (vertex*)linearAlloc(numvertices[ii] * sizeof(vertex));
			int idx = 0;
			for (unsigned int jj = 0; jj < grp.face_count; jj++)
			{
				unsigned int fv = mesh->face_vertices[grp.face_offset + jj];
				for (unsigned int kk = 0; kk < fv; kk++)
				{
					fastObjIndex mi = mesh->indices[grp.index_offset + idx];
					meshes[ii][idx] = {
						{
							mesh->positions[3 * mi.p + 0] * 0.01f, 
							mesh->positions[3 * mi.p + 1] * 0.01f, 
							mesh->positions[3 * mi.p + 2] * 0.01f
						}, 
						{
							mesh->texcoords[2 * mi.t + 0], 
							mesh->texcoords[2 * mi.t + 1]
						}, 
						{
							mesh->normals[3 * mi.n + 0], 
							mesh->normals[3 * mi.n + 1], 
							mesh->normals[3 * mi.n + 2]
						}
					};
					idx++;
				}
			}
		}
	}
	
	// Load the vertex shader, create a shader program and bind it
	vshader_dvlb = DVLB_ParseFile((u32*)fragvshader_shbin, fragvshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
	C3D_BindProgram(&program);

	// Get the location of the uniforms
	uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
	uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
	// uLoc_lightVec     = shaderInstanceGetUniformLocation(program.vertexShader, "lightVec");
	// uLoc_lightHalfVec = shaderInstanceGetUniformLocation(program.vertexShader, "lightHalfVec");
	// uLoc_lightClr     = shaderInstanceGetUniformLocation(program.vertexShader, "lightClr");
	// uLoc_material     = shaderInstanceGetUniformLocation(program.vertexShader, "material");
	// texcoord_offsets  = shaderInstanceGetUniformLocation(program.vertexShader, "texcoordoffsets");

	// Configure attributes for use with the vertex shader
	C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
	AttrInfo_Init(attrInfo);
	AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
	AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
	AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2=normal

	// Create the VBO (vertex buffer object)
	bufPlaza = new C3D_BufInfo[numgroups];
	for (unsigned int i = 0; i < numgroups; i++) {
		BufInfo_Init(&bufPlaza[i]);
		BufInfo_Add(&bufPlaza[i], meshes[i], sizeof(vertex), 3, 0x210);
	}

	// Load the texture and bind it to the first texture unit
	if (!loadTextureFromFile(&bottom_tex, NULL, "romfs:/gfx/kitten.t3x", true))
		svcBreak(USERBREAK_PANIC);
	C3D_TexSetFilter(&bottom_tex, GPU_LINEAR, GPU_NEAREST);
	C3D_TexSetWrap(&bottom_tex, GPU_REPEAT, GPU_REPEAT);

	C3D_LightEnvInit(&lightEnv);
	C3D_LightEnvBind(&lightEnv);
	C3D_LightEnvMaterial(&lightEnv, &lmat);

	LightLut_Phong(&lut_Light, 300);
	C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lut_Light);

	C3D_FVec lightVec = FVec4_New(0.0f, 0.0f, -0.5f, 1.0f);

	C3D_LightInit(&light, &lightEnv);
	C3D_LightColor(&light, 0.992, 0.984, 0.827);
	C3D_LightPosition(&light, &lightVec);


	r_act_on_objects(&root, &GameObject::Start); // start all scripts
}

void Scene1::update() {
	r_act_on_objects(&root, &GameObject::Update); // call update() on every gameobject (propagates from root)

	// whatever other per frame logic stuff will get called here

	r_act_on_objects(&root, &GameObject::LateUpdate); // call lateupdate() on every gameobject (propagates from root). Used to ensure stuff like cameras move only when everything else is done moving

	Console::update();
};



void Scene1::drawTop(float iod)
{
	Mtx_PerspStereoTilt(&projection, C3D_AngleFromDegrees(55.0f), C3D_AspectRatioTop, 0.01f, 1000.0f, iod, 2.0f, false);

	C3D_Mtx view = *objects.try_get<transform>(script1object);
	

	// Calculate the modelView matrix
	C3D_Mtx modelView;

	for (unsigned int i = 0; i < numgroups; i++) {
		C3D_LightEnvBind(&lightEnv);

		C3D_SetBufInfo(&bufPlaza[i]);

		C3D_TexBind(0, &bottom_tex);
		C3D_TexBind(1, &bottom_tex);

		C3D_TexEnv* env = C3D_GetTexEnv(0);
		C3D_TexEnvInit(env);
		C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_FRAGMENT_PRIMARY_COLOR);
		C3D_TexEnvFunc(env, C3D_Both, GPU_ADD_MULTIPLY);
		env = C3D_GetTexEnv(1);
		C3D_TexEnvInit(env);
		C3D_TexEnvSrc(env, C3D_Both, GPU_PREVIOUS, GPU_FRAGMENT_SECONDARY_COLOR);
		C3D_TexEnvFunc(env, C3D_Both, GPU_ADD);
		

		Mtx_Identity(&modelView);
		Mtx_Multiply(&modelView, &view, &modelView);
		

		// // Update the uniforms
		C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
		C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  &modelView);
		// C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_material,   &material);
		// C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightVec,     0.0f, 0.0f, -1.0f, 0.0f);
		// C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightHalfVec, 0.0f, 0.0f, -1.0f, 0.0f);
		// C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightClr,     1.0f, 1.0f,  1.0f, 1.0f);
		

		// // Draw the VBO
		C3D_DrawArrays(GPU_TRIANGLES, 0, numvertices[i]);
		_drawcalls++;
	}
}



void Scene1::drawBottom() {}

Scene1::~Scene1() {
	// Free the texture
	C3D_TexDelete(&bottom_tex);
	C3D_TexDelete(&top_tex);

	// Free the VBO
	for (unsigned int i = 0; i < numgroups; i++) linearFree(meshes[i]);

	delete[] meshes;

	// Free the shader program
	shaderProgramFree(&program);
	DVLB_Free(vshader_dvlb);
}