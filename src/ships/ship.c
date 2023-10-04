#include "common.h"
#include "util/iarray.h"
#include "gfx/polygon.h"
#include "gfx/model.h"
#include "physics.h"
#include "ship.h"
#include "util/maths.h"

static void centre_of_mass(int vertc, float* verts, vec2 out);

static intptr shipc;
static struct IArray ships;

void ships_init()
{
	shipc = 0;
	ships = iarr_new(sizeof(struct Ship), 4);
}

ShipID ship_new(enum ShipType type)
{
	struct Ship ship = {
		.type = type,
		.body = (struct Body){ .m = 100.0 },
	};

	/* Model */
	char buf[256];
	snprintf(buf, 256, MODEL_PATH "%s", STRING_OF_SHIPTYPE(type));
	struct Model mdl = model_new(buf);
	renderer_add_model(mdl);

	/* Centre of mass and moment of inertia */
	centre_of_mass(mdl.vertc, mdl.verts, ship.body.cm);
	ship.body.I = polygon_moment(mdl.vertc, mdl.verts, ship.body.cm, ship.body.m);

	/* Thrusters */
	switch (type) {
		case SHIPTYPE_1:
			ship.thrusterc = 1;
			ship.thrusters[0].Fmax = 1.0;
			ship.thrusters[0].F = 1.0;
			break;
		default:
			ERROR("ERROR");
	}

	shipc++;
	iarr_append(&ships, shipc, &ship);

	return shipc;
}

void ships_update()
{
	/* Update the model matrix for each ship */
	mat4* mat;
	struct Ship* ship;
	for (int i = 0; i < shipc; i++) {
		ship = (struct Ship*)ships.data + i;
		for (int j = 0; j < ship->thrusterc; j++)
			if (ship->thrusters[j].F > FLT_EPSILON)
				physics_apply_thrust(&ship->thrusters[j], &ship->body);

		physics_integrate(&ship->body);

		mat = renmats + ship->mdli;
		glm_mat4_identity(*mat);
		glm_rotate(*mat, ship->body.θ, (vec3){ 0.0, 0.0, 1.0 });
		glm_translate(*mat, (vec3){ ship->body.s[0], ship->body.s[1], 0.0 });
	}
}

void ships_free()
{
	intptr mdli;
	for (int i = 0; i < shipc; i++) {
		mdli = ((struct Ship*)ships.data)[i].mdli;
		renmdls[mdli].vertc = 0;
		vbo_free(&renmdls[mdli].vbo);
	}
	iarr_free(&ships, NULL);
}

static void centre_of_mass(int vertc, float* verts, vec2 out)
{
	out[0] = 0.0;
	out[1] = 0.0;
	for (int i = 0; i < vertc/2; i++) {
		out[0] += verts[2*i    ];
		out[1] += verts[2*i + 1];
	}
	out[0] /= vertc;
	out[1] /= vertc;
}
