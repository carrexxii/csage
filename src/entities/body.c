#include "components.h"
#include "systems.h"
#include "gfx/polygon.h"

static float verts[2*BODY_MAX_POLYGONS*MAX_VERTICES_PER_POLYGON];

struct Body body_new(int polyc, struct Polygon* polys, vec2 s, float m)
{
	struct Body body = {
		.s[0] = s[0],
		.s[1] = s[1],
		.m    = m,
	};

	/* Get the moment of inertia and centre of mass */
	int  vertc = 0;
	vec2 total = { 0, 0 };
	for (int i = 0; i < polyc; i++) {
		memcpy(verts + vertc, polys[i].verts, polys[i].vertc*sizeof(float[2]));
		for (int j = 0; j < polys[i].vertc; j++) {
			total[0] += polys[i].verts[2*j];
			total[1] += polys[i].verts[2*j + 1];
		}
		vertc += polys[i].vertc;
	}
	body.cm[0] = total[0]/vertc;
	body.cm[1] = total[1]/vertc;
	body.I = polygon_moment(vertc, verts, body.cm, m);

	return body;
}
