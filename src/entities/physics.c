#include "cglm/cglm.h"

#include "entity.h"
#include "components.h"
#include "systems.h"

void set_entity_pos(Entity e, vec3 pos)
{
	if (!(entities[e] & COMPONENT_MODEL))
		ERROR("[ENT] Entity %lu has not model component", e);

	// mat4* mat = iarr_get(components.mats, e);
	// glm_translate(*mat, pos);

	float* mat = iarr_get(components.mats, e);
	mat[12] = pos[0];
	mat[13] = pos[1];
	mat[14] = pos[2];
}
