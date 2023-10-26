#include "util/varray.h"
#include "gfx/model.h"
#include "body.h"
#include "pathfinding.h"
#include "ai.h"
#include "entity.h"

#define ENTITY(e)           (((struct Entity*)entities.data)[e - 1])
#define ENTITY_TRANSFORM(e) (mat4s*)(transforms.data + ((e) - 1)*sizeof(mat4s))
// #define ENTITY_TRANSFORM(e) (((mat4s*)transforms.data)[e - 1])
#define ENTITY_MODEL(e)     (((struct Model*)models.data)[e - 1])
#define ENTITY_BODY(e)      (((struct Body*)bodies.data)[e - 1])
#define ENTITY_PATH(e)      (((struct Path*)paths.data)[e - 1])
#define ENTITY_AI(e)        (((struct AI*)ais.data)[e - 1])

// Not sure if this is actually neccessary
struct Entity {
	EntityID transform;
	EntityID model;
	EntityID body;
	EntityID path;
	EntityID ai;
};

static void entity_update_transforms(SBO sbo_buf);

static struct VArray entities;
static struct VArray transforms;
static struct VArray models;
static struct VArray bodies;
static struct VArray paths;
static struct VArray ais;

void entity_init()
{
	entities   = varray_new(STARTING_ARRAY_SIZE, sizeof(struct Entity));
	transforms = varray_new(STARTING_ARRAY_SIZE, sizeof(mat4s));
	models     = varray_new(STARTING_ARRAY_SIZE, sizeof(struct Model*));
	bodies     = varray_new(STARTING_ARRAY_SIZE, sizeof(struct Body));
	paths      = varray_new(STARTING_ARRAY_SIZE, sizeof(struct Path));
	ais        = varray_new(STARTING_ARRAY_SIZE, sizeof(struct AI));

	update_model_transforms = entity_update_transforms;
	model_transforms = (mat4s*)transforms.data;
}

EntityID entity_new(vec3s pos, char* model_path)
{
	struct Model* model = model_new(model_path, false);
	struct Body body = {
		.pos     = pos,
		.facing  = (vec3s){ 1.0, 0.0, 0.0 },
		.vel     = 0.1,
		// .moving  = true,
	};
	struct Entity entity = {
		.transform = varray_push(&transforms, GLM_MAT4_IDENTITY),
		.model     = varray_push(&models, &model),
		.body      = varray_push(&bodies, &body),
		.path      = varray_push(&paths, &(struct Path){ 0 }),
		.ai        = varray_push(&ais, &(struct AI){ 0 }),
	};

	return varray_push(&entities, &entity) + 1;
}

void entity_path_to(EntityID e, ivec3s pos)
{
	struct Entity entity = ENTITY(e);
	struct Path* path = &ENTITY_PATH(entity.path);
	path->start = ivec3s_of_vec3s(ENTITY_BODY(entity.body).pos);
	path->end   = pos;

	path_new(path);
}

void entity_update()
{
	bodies_update(bodies.len, (struct Body*)bodies.data);
}

// TODO: Probably move this into the body update?
static void entity_update_transforms(SBO sbo_buf)
{
	mat4* transform;
	mat4 trans;
	struct Body* body = (struct Body*)bodies.data;
	for (int i = 1; i <= bodies.len; body++, i++) {
		glm_translate_make(trans, body->pos.raw);
		glm_rotate_z(trans, atan2f(body->facing.y, body->facing.x) - GLM_PI_2, trans);
		glm_rotate_x(trans, -GLM_PI_2, trans); // TODO: Move the rotation to model loading
		memcpy(ENTITY_TRANSFORM(i), trans, sizeof(mat4));
	}

	void* mem;
	intptr mem_size = transforms.len*sizeof(mat4);
	buffer_map_memory(sbo_buf, mem_size, &mem);
	memcpy(mem, transforms.data, mem_size);
	buffer_unmap_memory(sbo_buf);
}

void entity_free()
{
	varray_free(&entities);
	varray_free(&transforms);
	varray_free(&models);
	varray_free(&bodies);
	varray_free(&paths);
	varray_free(&ais);
}
