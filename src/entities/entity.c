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
	transforms = component_new(sizeof(mat4));
	models     = component_new(sizeof(struct Model*));

	update_model_transforms = entity_update_transforms;
}

Entity entity_new(vec3s pos, char* model_path)
{
	Entity e = ++entityc;

	mat4 transform = GLM_MAT4_IDENTITY_INIT;
	glm_translate(transform, pos.raw);
	glm_rotate(transform, -GLM_PI_2, (vec3){ 1.0, 0.0, 0.0 });
	component_add(&transforms, e, transform);

	struct Model* model = model_new(model_path, false);
	component_add(&models, e, &model);

	struct Body body;
	body.pos = pos;
	component_add(&bodies, e, &body);

	return e;
}

void entity_update()
{

}

static void entity_update_transforms(SBO sbo_buf)
{
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
