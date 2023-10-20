#include "gfx/model.h"
#include "body.h"
#include "component.h"
#include "entity.h"

#define MAX_ENTITIES (UINT16_MAX - 1)

static void entity_update_transforms(SBO sbo_buf);

static int    entityc;
static uint64 entities[MAX_ENTITIES];
static struct ComponentArray transforms;
static struct ComponentArray models;
static struct ComponentArray bodies;

void entity_init()
{
	transforms = component_new(sizeof(mat4s));
	models     = component_new(sizeof(struct Model*));
	bodies     = component_new(sizeof(struct Body));

	update_model_transforms = entity_update_transforms;
}

Entity entity_new(vec3s pos, char* model_path)
{
	Entity e = ++entityc;

	component_add(&transforms, e, GLM_MAT4_IDENTITY);

	struct Model* model = model_new(model_path, false);
	component_add(&models, e, &model);

	struct Body body = {
		.pos     = pos,
		.facing  = (vec3s){ 1.0, 0.0, 0.0 },
		.vel     = 0.1,
		// .moving  = true,
	};
	component_add(&bodies, e, &body);

	return e;
}

void entity_update()
{
	bodies_update(&bodies);
}

static void entity_update_transforms(SBO sbo_buf)
{
	struct Body* body;
	mat4* transform;
	mat4 t;
	uint16 e;
	FOREACH_COMPONENT2(bodies, transforms, e, body, transform,
		glm_translate_make(t, body->pos.raw);
		glm_rotate_z(t, atan2f(body->facing.y, body->facing.x) - GLM_PI_2, t);
		glm_rotate_x(t, -GLM_PI_2, t); // TODO: Move the rotation to model loading
		memcpy(*transform, t, sizeof(mat4));
	);

	void* mem;
	buffer_map_memory(sbo_buf, transforms.itemc*sizeof(mat4), &mem);
	component_copy(&transforms, mem);
	buffer_unmap_memory(sbo_buf);
}

void entity_free()
{
	component_free(&transforms, NULL);
	component_free(&models, NULL);
}
