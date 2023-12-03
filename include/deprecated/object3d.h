#pragma once

#include "entt.hpp"
#include "mesh.h"
#include "material.h"
#include "citro3d.h"
#include "defines.h"
#include "scene1.h"
#include "scenemanager.h"


class object3d {
    protected:
        entt::entity id;
        entt::registry &reg;

    public:
    object3d(entt::registry &r, mesh *m, material *mat) : reg(r) {
        this->id = r.create();
        reg.emplace<mesh>(this->id, *m);
        // reg.emplace<material>(this->id, *mat);
    }

	~object3d() {
        delete reg.try_get<mesh>(id);
        delete reg.try_get<C3D_Mtx>(id);
        if (reg.orphan(id)) reg.storage<entt::entity>().erase(id);
	}

    mesh *Mesh() {
        return reg.try_get<mesh>(id);
    }

    void draw(C3D_Mtx *view) {
		material *mat = reg.try_get<material>(id);
        mesh *m = reg.try_get<mesh>(id);
        if (!m || !mat) {
            #if CONSOLE_ENABLED
            printf("Unable to retrieve either material or mesh on object with id %lu", (uint32_t)id);
            #endif
        } else {
			C3D_SetBufInfo(m->buf);
			// mat->setMaterial(view);

			// Draw the VBO
			C3D_DrawArrays(GPU_TRIANGLES, 0, m->numVerts);
		}
    }
};