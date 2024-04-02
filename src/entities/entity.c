#include "maths/maths.h"
#include "util/varray.h"
#include "body.h"
#include "pathfinding.h"
#include "ai.h"
#include "entity.h"

#define ENTITY(e)      (((struct Entity*)entities.data)[e - 1])
#define ENTITY_BODY(e) (((struct Body*)bodies.data)[e])
#define ENTITY_PATH(e) (((struct Path*)paths.data)[e])
#define ENTITY_AI(e)   (((struct AI*)ais.data)[e])

// Not sure if this is actually neccessary
struct Entity {
	Entity body;
	Entity path;
	Entity ai;
};

static void entity_update_transforms(SBO sbo_buf);

static struct VArray entities;
static struct VArray transforms;
static struct VArray bodies;
static struct VArray paths;
static struct VArray ais;

void entities_init()
{
	entities = varray_new(STARTING_ARRAY_SIZE, sizeof(struct Entity));
	bodies   = varray_new(STARTING_ARRAY_SIZE, sizeof(struct Body));
	paths    = varray_new(STARTING_ARRAY_SIZE, sizeof(struct Path));
	ais      = varray_new(STARTING_ARRAY_SIZE, sizeof(struct AI));
}

// TODO: make sheet a string paramter and add spritesheets to resmgr
Entity entity_new(struct SpriteSheet* sheet, Vec2 pos)
{
	struct Body body = {
		.pos     = pos,
		.facing  = VEC2(1.0f, 0.0f),
		.vel     = 0.1f,
	};
	struct Entity entity = {
		.body = varray_push(&bodies, &body),
		.path = varray_push(&paths, &(struct Path){ .complete = true }),
		.ai   = varray_push(&ais, &(struct AI){ 0 }),
	};

	return varray_push(&entities, &entity) + 1;
}

void entity_path_to(Entity e, Vec3i pos)
{
	struct Entity entity = ENTITY(e);
	struct Path* path = &ENTITY_PATH(entity.path);
	// path->start = VEC2I_V(ENTITY_BODY(entity.body).pos);
	path->end   = pos;
	// path_new(path);
}

void entities_update()
{
	struct Body* body;
	struct Path* path;

	struct Entity* entity = (struct Entity*)entities.data;
	for (int i = 0; i < entities.len; entity++, i++) {
		path = &ENTITY_PATH(entity->path);
		if (!path->complete) {
			body = &ENTITY_BODY(entity->body);

			Vec3 current_dest = VEC3_A(path->local_path[path->local_path_current]);
			if (distance2(body->pos, VEC2_V3(path->end)) > ENTITY_PATH_EPSILON) {
				// if (distance2(body->pos, current_dest) <= ENTITY_PATH_EPSILON) {
				// 	path->local_path_current++;
				// 	current_dest = VEC3_A(path->local_path[path->local_path_current]);
				// }
				// body->moving = true;
				// body->facing = sub(current_dest, body->pos);
				// body->facing.z = 0.0;
				// body->facing = normalized(body->facing);
			} else {
				path->complete = true;
				body->moving   = false;
			}

		}
	}

	bodies_update(bodies.len, (struct Body*)bodies.data);
	// ais_update(ais.len, (struct AI*)ais.data);
}

// TODO: Probably move this into the body update?
static void entity_update_transforms(SBO sbo_buf)
{
	Mat4x4 trans;
	struct Body* body = (struct Body*)bodies.data;
	for (int i = 0; i <= bodies.len; body++, i++) {
		// static int c;
		// body->facing.x = sinf((float)c/1000.0);
		// body->facing.y = cosf((float)c/1000.0);
		// body->facing.z = cosf((float)c/1000.0);
		// c++;

		// glm_translate_make(trans, body->pos.raw);
		// glm_rotate_z(trans, atan2f(body->facing.y, body->facing.x) - PI, trans);
		// glm_rotate_x(trans, -PI_2, trans); // TODO: Move the rotation to model loading
		// // glm_rotate_x(trans, fmod(body->facing.z, (2.0*GLM_PI)), trans);
		// glm_scale(trans, (vec3){ 1.0, 1.0, 1.0 });
		// memcpy(ENTITY_TRANSFORM(i), trans, sizeof(Mat4x4));
	}

	buffer_update(sbo_buf, transforms.len*sizeof(Mat4x4), transforms.data, 0);
}

void entities_free()
{
	varray_free(&entities);
	varray_free(&bodies);
	varray_free(&paths);
	varray_free(&ais);
}
