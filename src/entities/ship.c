#include "util/iarray.h"
#include "gfx/polygon.h"
#include "gfx/model.h"
#include "physics.h"
#include "ship.h"

static void centre_of_mass(int vertc, float* verts, vec2 out);

static struct Body shipbodies[SHIP_TYPE_COUNT];
static intptr shipc;
static struct IArray ships;
static float shipverts[SHIP_TYPE_COUNT][256] = {
	{ NAN },                                /* SHIPTYPE_NONE */
	{ -0.2, 0.0, 0.0, 0.6, 0.2, 0.0, NAN }, /* SHIPTYPE_1 */
};

void ships_init()
{
	shipc = 0;
	ships = iarr_new(sizeof(struct Ship), 4);

	int    vertc = sizeof(shipverts[SHIPTYPE_1])/(2*sizeof(**shipverts));
	float* verts = shipverts[SHIPTYPE_1];
	shipbodies[SHIPTYPE_1] = (struct Body){
		.m = 100.0,
	};
	centre_of_mass(vertc, verts, shipbodies[SHIPTYPE_1].cm);
	shipbodies[SHIPTYPE_1].I = polygon_moment(vertc, verts, shipbodies[SHIPTYPE_1].cm, shipbodies[SHIPTYPE_1].m);
}

ShipID ship_new(enum ShipType type)
{
	struct Ship ship = {
		.type = type,
		.body = shipbodies[type],
	};

	shipc++;
	iarr_append(&ships, shipc, &ship);

	struct Model mdl = polygons_to_model(1, (struct Polygon[]){ polygon_new(shipverts[type], (vec3){ COLOUR_RED }) }, false);
	renderer_add_model(mdl);

	return shipc;
}

void ship_add_body(ShipID id, enum ShipType type)
{

}

void ships_update()
{
	/* Update the model matrix for each ship */
	mat4* mat;
	struct Ship* ship;
	for (int i = 0; i < shipc; i++) {
		ship = (struct Ship*)ships.data + i;
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
	for (int i = 0; i < vertc; i++) {
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
