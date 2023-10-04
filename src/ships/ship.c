#include "common.h"
#include "util/iarray.h"
#include "gfx/polygon.h"
#include "gfx/model.h"
#include "physics.h"
#include "ship.h"

static void centre_of_mass(int vertc, float* verts, vec2 out);

static struct Body shipbodies[SHIP_TYPE_COUNT];
static intptr shipc;
static struct IArray ships;

void ships_init()
{
	shipc = 0;
	ships = iarr_new(sizeof(struct Ship), 4);

	// centre_of_mass(vertc, verts, shipbodies[SHIPTYPE_1].cm);
	// shipbodies[SHIPTYPE_1].I = polygon_moment(vertc, verts, shipbodies[SHIPTYPE_1].cm, shipbodies[SHIPTYPE_1].m);
}

ShipID ship_new(enum ShipType type)
{
	struct Ship ship = {
		.type = type,
		.body = shipbodies[type],
	};

	/* Model */
	char buf[256];
	snprintf(buf, 256, MODEL_PATH "%s", STRING_OF_SHIPTYPE(type));
	struct Model mdl = model_new(buf);
	renderer_add_model(mdl);

	/* Thrusters */
	// struct Thruster** ts = &ship.thrusters;
	// int tc = 0;
	// float* tdata;
	// do {
	// 	*ts = smalloc(sizeof(struct Thruster));
	// 	**ts = (struct Thruster) {
	// 		.s[0] = tdata[0],
	// 		.s[1] = tdata[1],
	// 		.θ    = tdata[2],
	// 		.Fmin = tdata[3],
	// 		.Fmax = tdata[4],
	// 	};
	// 	tc++;
	// } while (!isnan(tdata[5*tc]));

	shipc++;
	iarr_append(&ships, shipc, &ship);

	return shipc;
}

void ships_update()
{
	/* Update the model matrix for each ship */
	mat4* mat;
	struct Ship* ship;
	// struct Thruster* thruster;
	for (int i = 0; i < shipc; i++) {
		ship = (struct Ship*)ships.data + i;
		// thruster = ship->thrusters;
		// while (thruster) {
		// 	if (thruster->F > FLT_EPSILON)
		// 		physics_apply_thrust(*thruster, &ship->body);
		// 	thruster = thruster->next;
		// }

		physics_integrate(&ship->body);

		mat = renmats + ship->mdli;
		glm_mat4_identity(*mat);
		glm_rotate(*mat, ship->body.θ, (vec3){ 0.0, 0.0, 1.0 });
		glm_translate(*mat, (vec3){ ship->body.s[0], ship->body.s[1], 0.0 });
	}
	// mat4 mat;
	// struct Body* body = components.bodies.data;
	// for (int i = 0; i < components.bodies.itemc; body++, i++) {
	// 	glm_mat4_identity(mat);
	// 	// ********************************************************************
	// 	     if (body->s[0] >  3.0) body->s[0] = -3.0;
	// 	else if (body->s[0] < -3.0) body->s[0] =  3.0;
	// 	else if (body->s[1] >  3.0) body->s[1] = -3.0;
	// 	else if (body->s[1] < -3.0) body->s[1] =  3.0;
	// 	// ********************************************************************
	// 	glm_rotate(mat, body->θ, (vec3){ 0.0, 0.0, 1.0 });
	// 	glm_translate(mat, (vec3){ body->s[0], body->s[1], 0.0 });
	// 	glm_mat4_ucopy(mat, ((mat4*)components.mats.data)[i]);
	// }
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

// #define THRUSTER_COLOUR 0.7, 0.0, 0.0

// void ship_add_thruster_model(struct Thruster* thruster)
// {
// 	float w = thruster->Fmax;
// 	float h = w/8.0;
// 	float x = thruster->s[0];
// 	float y = thruster->s[1];
// 	float verts[] = {
// 		x - w, y + h, THRUSTER_COLOUR,
// 		x + w, y + h, THRUSTER_COLOUR,
// 		x - w, y - h, THRUSTER_COLOUR,

// 		x - w, y - h, THRUSTER_COLOUR,
// 		x + w, y + h, THRUSTER_COLOUR,
// 		x + w, y - h, THRUSTER_COLOUR,
// 	};

// 	struct Model mdl = {
// 		.vertc = 6,
// 		.vbo   = vbo_new(sizeof(verts), verts),
// 	};
// 	thruster->mdl = iarr_append(&components.mdls, UINT16_MAX, &mdl);
// }
