#include "util/iarray.h"
#include "gfx/model.h"
#include "components.h"
#include "gfx/renderer.h"
#include "camera.h"
#include "systems.h"
#include "entity.h"

intptr entityc;
Entity entities[MAX_ENTITIES];
struct EntityComponents components;

void entities_init()
{
	components.mdls   = iarr_new(sizeof(struct Model), 10);
	components.mats   = iarr_new(sizeof(mat4), 10);
	components.lights = iarr_new(sizeof(vec4), 5);
	components.bodies = iarr_new(sizeof(struct Body), 10);
	components.actors = iarr_new(sizeof(struct Actor), 10);

	renmdlc = &components.mdls.itemc;
	renmdls = components.mdls.data;
	renmats = components.mats.data;

	DEBUG(1, "[ENT] Initialized entities");
}

Entity entity_new()
{
	entities[++entityc] = COMPONENT_NONE;
	return entityc;
}

void entity_add_component(Entity e, enum Component c, void* data)
{
	struct Model mdl;
	vec4* light;
	struct Actor actor;
	switch (c) {
		case COMPONENT_NONE:
			ERROR("[ENT] Trying to add COMPONENT_NONE");
			break;
		case COMPONENT_MODEL:
			mdl = create_model((char*)data);
			iarr_append(&components.mdls, e, &mdl);
			iarr_append(&components.mats, e, GLM_MAT4_IDENTITY);
			break;
		case COMPONENT_LIGHT:
			light = iarr_append(&components.lights, e, data);
			renderer_add_light(*light);
			break;
		case COMPONENT_BODY:
			glm_vec3_copy(((struct Body*)data)->pos, ((struct Body*)data)->prevpos);
			iarr_append(&components.bodies, e, data);
			entity_set_body_dimensions(e);
			break;
		case COMPONENT_ACTOR:
			actor = (struct Actor){ 0 };
			iarr_append(&components.actors, e, &actor);
			break;
		case COMPONENT_CONTROLLABLE:
			break;
		default:
			ERROR("[ENT] Unhandled component add: %u", c);
	}
	entities[e] |= c;
}

void entity_move(Entity e, vec3 dir)
{
	struct Body* body = (struct Body*)iarr_get(components.bodies, e);
	body->forcec = 0;
	glm_vec3_copy(dir, body->forces[body->forcec++]);
}

bool entity_select_by_point(vec3 p)
{
	struct Body* body = components.bodies.data;
	for (int i = 0; i < components.bodies.itemc; i++, body++) {
		float rect[] = { body->pos[0] - body->r, body->pos[1] + body->h/2.0,
		                 body->pos[0] + body->r, body->pos[1]};
		glm_vec3_print(p, stderr);
		glm_vec4_print(rect, stderr);
		DEBUG(1, "%d", point_in_rect(p, rect));
	}
	
	return false;
}

void entity_set_body_dimensions(Entity e)
{
	struct Model* mdl = iarr_get(components.mdls, e);
	struct Body* body = iarr_get(components.bodies, e);
	if (!body) {
		DEBUG(1, "[ENT] Entity %ld does not have a Body component", e);
		return;
	}
	if (!mdl) {
		DEBUG(1, "[ENT] !!Warning, entity %ld does not have dimensions (no respective model)!!", e);
	} else {
		body->r = MIN(mdl->dim[0], mdl->dim[1]);
	}
}

void entities_update()
{
	physics_integrate();
	physics_resolve_collisions();

	actors_update();

	/* Update the model matrices */
	vec3 pos;
	mat4 transform;
	struct Body* body = components.bodies.data;
	for (int i = 0; i < components.mats.itemc; i++) {
		glm_vec3_copy(body->pos, pos);
		pos[2] -= camzlvl; // IMPROVEMENT: Maybe better way to do this?
		glm_translate_make(transform, pos);
		glm_rotate(transform, glm_rad(180.0), (vec3){ 0.0, 1.0, 0.0 }); // TODO: do this in the exporter instead
		glm_rotate(transform, -body->dir, (vec3){ 0.0, 0.0, 1.0 });
		glm_mat4_ucopy(transform, ((mat4*)components.mats.data)[i]);
	}
}

void entities_free()
{
	DEBUG(1, "[ENT] Freeing entities...");
	iarr_free(&components.mdls  , (void (*)(void*))free_model);
	iarr_free(&components.mats  , NULL);
	iarr_free(&components.lights, NULL);
	iarr_free(&components.bodies, NULL);
	iarr_free(&components.actors, (void (*)(void*))actor_free);
}

