#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <tex3ds.h>
#include <string.h>
#include "vshader_shbin.h"
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
// #include "movement_script.h"
#include "componentmanager.h"
#include "script1.h"

#define vertex_list_count (sizeof(vertex_list)/sizeof(vertex_list[0]))

#define M_RAD 0.01745329252f

static unsigned int *numvertices = nullptr, numgroups = 0;
// static char text[128];

void drawText(float x, float y, float z, float scale, const char* value) {
	C2D_Prepare();
    C2D_TextBuf buf=C2D_TextBufNew(4096);
    C2D_Text text;
    C2D_TextParse(&text, buf, value);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, 0, x, y, z, scale, scale);
	C2D_Flush();
    C2D_TextBufDelete(buf);
}

// Helper function for loading a texture from memory
bool loadTextureFromMem(C3D_Tex* tex, C3D_TexCube* cube, const void* data, size_t size)
{
	Tex3DS_Texture t3x = Tex3DS_TextureImport(data, size, tex, cube, false);
	if (!t3x){
		printf("didn't find it");
		return false;}

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

int draw(entt::registry registry, entt::entity object) {
	// Mesh<vertex> *mesh = registry.try_get<Mesh<vertex>>(object); 
	// if (!mesh) return 1;
	// material *mat = registry.try_get<material>(object);
	// if (!mat) return 2;

	// void* vbo_data = linearAlloc()


	return 0;
}

void printmesh(const Mesh *mesh) {
	if (mesh == NULL) return;
	#if CONSOLE_ENABLED
	for (int i = 0; i < mesh->numVerts; i++) printf("%u. pos %f %f %f\n", i, ((vertex*)mesh->vertices)[i].position[0], ((vertex*)mesh->vertices)[i].position[1], ((vertex*)mesh->vertices)[i].position[2]);
	#endif
}

void printmesh(const void *mesh, const int n) {
	if (mesh == NULL) return;
	#if CONSOLE_ENABLED
	for (int i = 0; i < n; i++) printf("%u. pos %f %f %f\n", i, ((vertex*)mesh)[i].position[0], ((vertex*)mesh)[i].position[1], ((vertex*)mesh)[i].position[2]);
	#endif
}

char* getfiletext(const char* path)
{
	FILE* f = fopen(path, "r");
	char* buffer;
	std::ifstream t;
	int length;
	t.open(path);      // open input file
	t.seekg(0, std::ios::end);    // go to the end
	length = t.tellg();           // report location (this is the length)
	t.seekg(0, std::ios::beg);    // go back to the beginning
	buffer = new char[length];    // allocate memory for a buffer of appropriate dimension
	t.read(buffer, length);       // read the whole file into the buffer
	t.close(); 
	if (f)
	{
		
		fgets(buffer, length, f);
		fclose(f);
	}
	return buffer;
}

Scene1::Scene1() : Scene("Scene 1"), cube(objects.create()), camera(objects.create()), script1object(objects), obj(mdlLoader::load("romfs:/cube.slmdl", objects)) {
	objects.emplace<transform>(cube);
	objects.emplace<transform>(camera);

	// printf("Script1 registered: %s\n", Script1_component ? "true" : "false");

	ComponentManager::addComponent("transform", objects, script1object.id);
	script1object.scripts[0] = ComponentManager::addScript("Script1", objects, script1object.id);
	script1object.scripts[0]->Start();
	
	
	transform *cubepos = objects.try_get<transform>(cube), *campos = objects.try_get<transform>(camera);

	cubepos->position = {0.0f, -1.0f, 0.0f};
	campos->position = {0, 0, -4};
	mesh = fast_obj_read("romfs:/plaza.obj");

	// printmesh(obj->mesh());

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
	vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
	C3D_BindProgram(&program);

	// Get the location of the uniforms
	uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
	uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
	uLoc_lightVec     = shaderInstanceGetUniformLocation(program.vertexShader, "lightVec");
	uLoc_lightHalfVec = shaderInstanceGetUniformLocation(program.vertexShader, "lightHalfVec");
	uLoc_lightClr     = shaderInstanceGetUniformLocation(program.vertexShader, "lightClr");
	uLoc_material     = shaderInstanceGetUniformLocation(program.vertexShader, "material");
	texcoord_offsets  = shaderInstanceGetUniformLocation(program.vertexShader, "texcoordoffsets");

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

	// if (objects.try_get<Script1>(cube)) objects.try_get<Script1>(cube)->Update();
	// else printf("didn't find it\n");

	LightLut_Phong(&lut_Light, 300);
	C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lut_Light);

	C3D_FVec lightVec = FVec4_New(0.0f, 0.0f, -0.5f, 1.0f);

	C3D_LightInit(&light, &lightEnv);
	C3D_LightColor(&light, 1.0, 1.0, 1.0);
	C3D_LightPosition(&light, &lightVec);

}

void Scene1::update() {
	transform *cam = objects.try_get<transform>(camera);
	
	// float dTime = C3D_GetProcessingTime() * 0.001;
	cam->rotation.x = controls::gyroPos().x;
	cam->rotation.y = -controls::gyroPos().z;
	// float angle = atan2f(abs(controls::circlePos().dx) > 20 ? controls::circlePos().dx : 0, abs(controls::circlePos().dy) > 20 ? controls::circlePos().dy : 0) + cam->rotation.y;
	// float invmagnitude = 1 / sqrtf(controls::circlePos().dx * controls::circlePos().dx + controls::circlePos().dy * controls::circlePos().dy);

    cam->position.x += (abs(controls::circlePos().dx) > 20 ? controls::circlePos().dx : 0) * Time::deltaTime * 0.005f;
    cam->position.z += (abs(controls::circlePos().dy) > 20 ? controls::circlePos().dy : 0) * Time::deltaTime * 0.005f;
	cam->position.y += (controls::getHeld("L") ? 128 : controls::getHeld("R") ? -128 : 0) * Time::deltaTime * 0.005f;

	
	// float fps = C3D_GetProcessingTime();
	
	offsetX += 0.01f;
	offsetY += 0.01f;
	offset2 += 0.005f;
	if (controls::getHeld("dup")) angleY += 1;
	if (controls::getHeld("ddown")) angleY -= 1;
	if (controls::getHeld("dright")) angleX += 0.1f;
	if (controls::getHeld("dleft")) angleX -= 0.1f;
	if (controls::getHeld("a")) distance += 0.1f;
	if (controls::getDown("y")) controls::resetGyro({0, 0, 0});

	script1object.scripts[0]->Update();

	// 
	// printf("\x1b[29;1Hp=%2.1f %2.1f %2.1f r=%2.1f %2.1f %2.1f\n", campos->position.x, campos->position.y, campos->position.z, campos->rotation.x, campos->rotation.y, campos->rotation.z);
	// printf("\x1b[29;1Hp=%d,%d at=%d dTime: %1.2f \n", controls::circlePos().dx, controls::circlePos().dy, (unsigned char)angleY, C3D_GetProcessingTime());
	// printf("%f %f\n", sinf(angle) * dTime, cosf(angle) * dTime);

	Console::setFPS(1.0f / Time::deltaTime);
	Console::setFrameTime(Time::deltaTime * 1000);
	Console::setDrawCalls(numgroups);
	Console::setPosition(cam->position.x, cam->position.y, cam->position.z);
	Console::updateMemUsage();

	// snprintf(text, 128, "fps: %u", fps);
};



void Scene1::drawTop(float iod)
{
	Mtx_PerspStereoTilt(&projection, C3D_AngleFromDegrees(80.0f), C3D_AspectRatioTop, 0.01f, 1000.0f, iod, 2.0f, false);

	transform *campos = objects.try_get<transform>(camera);
	C3D_Mtx view;
	Mtx_Identity(&view);
	Mtx_RotateZ(&view, campos->rotation.z, true); // zxy rotation order, default in unity
	Mtx_RotateX(&view, campos->rotation.x, true);
	Mtx_RotateY(&view, campos->rotation.y, true);
	Mtx_Translate(&view, -campos->position.x, campos->position.y, campos->position.z, true);

	// Calculate the modelView matrix
	C3D_Mtx modelView;

	for (unsigned int i = 0; i < 1; i++) {
		

		C3D_SetBufInfo(&bufPlaza[i]);

		// C3D_SetBufInfo(obj->mesh()->buf);

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
		C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_material,   &material);
		C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightVec,     0.0f, 0.0f, -1.0f, 0.0f);
		C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightHalfVec, 0.0f, 0.0f, -1.0f, 0.0f);
		C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightClr,     1.0f, 1.0f,  1.0f, 1.0f);
		// C3D_FVUnifSet(GPU_VERTEX_SHADER, texcoord_offsets,     offsetX * 0.1f, offsetY * 0.1f,  offset2 * 0.1f, 1.0f);
		
		// obj->draw(&view);

		// // Draw the VBO
		C3D_DrawArrays(GPU_TRIANGLES, 0, numvertices[i]);
		// C3D_DrawArrays(GPU_TRIANGLES, 0, obj->mesh()->numVerts / 3);
	}
	
}



void Scene1::drawBottom() {

	// Draw the 2d scene
	// drawText(8, 8, 1, 1, text);
	
	// if (tb) {
	// 	char* text; 
	// 	// sprintf(text, "RAM Usage:%lu", (osGetMemRegionUsed(MEMREGION_APPLICATION) * 100) / osGetMemRegionSize(MEMREGION_APPLICATION));
	// 	C2D_TextParse(&t, tb, "text");

	// 	C2D_Prepare();
	// 	C2D_DrawText(&t, 0.0f, 8.0f, 8.0f, 1.0f, 1.0f, 1.0f);
	// 	C2D_Flush();
	// } 
	
}

Scene1::~Scene1() {
	// Free the texture
	C3D_TexDelete(&bottom_tex);
	C3D_TexDelete(&top_tex);

	// Free the VBO
	linearFree(vbo_data);
	linearFree(vbo_data2);

	delete[] meshes;

	// Free the shader program
	shaderProgramFree(&program);
	DVLB_Free(vshader_dvlb);
}