#include <stdarg.h>

#include "polygon.h"
#include "vertices.h"
#include "renderer.h"

inline static float* copy_triangle(float* v, float* origin, float* vertstart, vec3 colour);

struct Polygon polygon_new(float* verts, vec3 colour)
{
	struct Polygon poly = { .vertc = 0 };
	float* v = verts;
	while (!isnan(*v)) {
		poly.vertc++;
		v += 2;
	}

	poly.verts = smalloc(poly.vertc*sizeof(vec2)),
	memcpy(poly.verts, verts, poly.vertc*sizeof(vec2));
	memcpy(poly.colour, colour, sizeof(vec3));

	return poly;
}

struct Model polygons_to_model(int polyc, struct Polygon* polys, bool free_polys)
{
	struct Model mdl = { 0 };
	intptr memsz = 0;

	int tric;
	float* verts = smalloc(BODY_MAX_POLYGONS*MAX_VERTICES_PER_POLYGON*SIZEOF_VERTEX);
	float* v = verts;
	for (int i = 0; i < polyc; i++) {
		tric       = polys[i].vertc - 2;
		memsz     += 3*SIZEOF_VERTEX*tric;
		mdl.vertc += 3*tric;
		for (int j = 0; j < tric; j++)
			v = copy_triangle(v, polys[i].verts, polys[i].verts + 2*j, polys[i].colour);
	}

	mdl.vbo = vbo_new(memsz, verts);

	if (free_polys)
		for (int i = 0; i < polyc; i++)
			polygon_free(polys[i]);
	free(verts);
	return mdl;
}

void polygon_print(struct Polygon poly)
{
	DEBUG(1, "Polygon with %ld vertices: ", poly.vertc);
	for (int i = 0; i < poly.vertc; i++) {
		DEBUG(1, "  [%d] %.2f, %.2f", i, poly.verts[2*i], poly.verts[2*i+1]);
	}
}

void polygon_free(struct Polygon poly)
{
	free(poly.verts);
}

inline static float* copy_triangle(float* v, float* origin, float* vertstart, vec3 colour)
{
	// DEBUG(1, "%.2f, %.2f, %.2f, %.2f, %.2f, %.2f", origin[0], origin[1], vertstart[2], vertstart[3], vertstart[4], vertstart[5]);
	*v++ = origin[0];
	*v++ = origin[1];
	*v++ = colour[0];
	*v++ = colour[1];
	*v++ = colour[2];

	*v++ = vertstart[2];
	*v++ = vertstart[3];
	*v++ = colour[0];
	*v++ = colour[1];
	*v++ = colour[2];

	*v++ = vertstart[4];
	*v++ = vertstart[5];
	*v++ = colour[0];
	*v++ = colour[1];
	*v++ = colour[2];

	return v;
}
