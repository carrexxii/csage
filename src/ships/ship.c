#include "common.h"
#include "util/iarray.h"
#include "gfx/polygon.h"
#include "gfx/model.h"
#include "physics.h"
#include "ship.h"
#include "util/maths.h"

static void set_verts_from_model(struct Body* body, struct Model mdl);
static void set_centre_of_mass(struct Ship* ship);

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
		.body = (struct Body){ .m = 100.0, .θ = GLM_PI_2, },
	};

	/* Model */
	char buf[256];
	snprintf(buf, 256, MODEL_PATH "%s", STRING_OF_SHIPTYPE(type));
	struct Model mdl = model_new(buf);
	renderer_add_model(mdl);

	/* Centre of mass and moment of inertia */
	set_verts_from_model(&ship.body, mdl);
	set_centre_of_mass(&ship);
	ship.body.I = polygon_moment(mdl.vertc, mdl.verts, ship.body.cm, ship.body.m);

	/* Thrusters */
	switch (type) {
		case SHIPTYPE_1:
			ship.thruster.s[0]  = 0.0;
			ship.thruster.s[1]  = 0.0;
			ship.thruster.F_max = 50.0;
			ship.thruster.τ_max = 5.0;
			break;
		default:
			ERROR("No thruster values for ship %d", type);
	}

	shipc++;
	iarr_append(&ships, shipc, &ship);

	return shipc;
}

struct Ship* ship_get(ShipID ship)
{
	return iarr_get(ships, ship);
}

void ships_update()
{
	/* Update the model matrix for each ship */
	mat4* mat;
	struct Ship* ship;
	for (int i = 0; i < shipc; i++) {
		ship = (struct Ship*)ships.data + i;
		physics_apply_thrust(&ship->thruster, &ship->body);

		physics_integrate(&ship->body);

		mat = renmats + ship->mdli;
		glm_mat4_identity(*mat);
		glm_translate(*mat, (vec3){ ship->body.s[0], ship->body.s[1], 0.0 });
		glm_translate(*mat, (vec3){ ship->body.cm[0], ship->body.cm[1], 0.0 });
		glm_rotate(*mat, ship->body.θ - GLM_PI_2, (vec3){ 0.0, 0.0, 1.0 });
		glm_translate(*mat, (vec3){ -ship->body.cm[0], -ship->body.cm[1], 0.0 });
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

static void set_verts_from_model(struct Body* body, struct Model mdl)
{
	body->vertc = mdl.vertc;
	body->verts = smalloc(mdl.vertc*sizeof(vec2));
	for (int i = 0; i < mdl.vertc; i++) {
		body->verts[2*i    ] = mdl.verts[MODEL_VERTEX_ELEMENTS*i    ];
		body->verts[2*i + 1] = mdl.verts[MODEL_VERTEX_ELEMENTS*i + 1];
	}
}

static void set_centre_of_mass(struct Ship* ship)
{
	int vertc    = ship->body.vertc;
	float* verts = ship->body.verts;
	float* cm    = ship->body.cm;

	cm[0] = 0.0;
	cm[1] = 0.0;
	for (int i = 0; i < vertc; i++) {
		cm[0] += verts[2*i    ];
		cm[1] += verts[2*i + 1];
	}
	cm[0] /= vertc;
	cm[1] /= vertc;
}
