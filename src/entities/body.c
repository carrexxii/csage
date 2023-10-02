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

	/* Get the moment of inertia */
	int vertc = 0;
	for (int i = 0; i < polyc; i++) {
		memcpy(verts + vertc, polys[i].verts, polys[i].vertc*sizeof(float[2]));
		vertc += polys[i].vertc;
	}
	body.I = polygon_moment(vertc, verts, m);

	return body;
}
