#pragma once

#include "gameobject.h"
#include "fast_obj.h"
#include "mesh.h"
#include "entt.hpp"
#include "fragmentlit.h"

namespace objLoader {
    inline bool addModel(const char* path, GameObject& obj) {
        fastObjMesh* Mesh = fast_obj_read(path);
        if (Mesh->face_count == 0) return false;

        vertex* vmesh = NULL;
        const fastObjGroup& grp = Mesh->groups[0]; // only take the first one bc it's simpler
        unsigned int numvertices = grp.face_count * 3;
        vmesh = (vertex*)linearAlloc(numvertices * sizeof(vertex));
        int idx = 0;
        for (unsigned int jj = 0; jj < grp.face_count; jj++)
        {
            unsigned int fv = Mesh->face_vertices[grp.face_offset + jj];
            for (unsigned int kk = 0; kk < fv; kk++)
            {
                fastObjIndex mi = Mesh->indices[grp.index_offset + idx];
                vmesh[idx] = {
                    {
                        Mesh->positions[3 * mi.p + 0], 
                        Mesh->positions[3 * mi.p + 1], 
                        Mesh->positions[3 * mi.p + 2]
                    }, 
                    {
                        Mesh->texcoords[2 * mi.t + 0], 
                        Mesh->texcoords[2 * mi.t + 1]
                    }, 
                    {
                        Mesh->normals[3 * mi.n + 0], 
                        Mesh->normals[3 * mi.n + 1], 
                        Mesh->normals[3 * mi.n + 2]
                    }
                };
                idx++;
            }
        }
        obj.reg.emplace_or_replace<mesh>(obj.id);
        mesh* scmesh = obj.getComponent<mesh>();
        if (!scmesh) return false;
        object.reg.emplace_or_replace<MeshRenderer>(obj.id, obj, new fragment_lit());
        scmesh->vertices = vmesh;
        scmesh->numVerts = numvertices;
        return true;
    }
}