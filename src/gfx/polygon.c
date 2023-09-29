#include <stdarg.h>

#include "polygon.h"
#include "vertices.h"
#include "renderer.h"

struct Polygon polygon_new(int vertc, ...)
{
	struct Polygon poly = {
		.vertc = vertc,
		.verts = scalloc(4, 2*vertc),
	};

	va_list points;
	va_start(points, vertc);
	for (int i = 0; i < 2*vertc; i++)
		poly.verts[i] = va_arg(points, double);

	va_end(points);
	return poly;
}

struct Model polygon_to_model(struct Polygon poly, bool free_poly)
{
	struct Model mdl;
	int tric  = poly.vertc - 2;
	mdl.vertc = 3*tric;

	intptr memsz = SIZEOF_VERTEX*3*tric;
	float* verts = smalloc(memsz);
	float* v = verts;
	for (int i = 0; i < tric; i++) {
		*v++ = poly.verts[0];
		*v++ = poly.verts[1];
		*v++ = 1.0;
		*v++ = 1.0;
		*v++ = 1.0;
		*v++ = poly.verts[2*i + 2];
		*v++ = poly.verts[2*i + 3];
		*v++ = 1.0;
		*v++ = 1.0;
		*v++ = 1.0;
		*v++ = poly.verts[2*i + 4];
		*v++ = poly.verts[2*i + 5];
		*v++ = 1.0;
		*v++ = 1.0;
		*v++ = 1.0;
		DEBUG(1, "%f, %f, %f, %f, %f, %f", poly.verts[0], poly.verts[1], poly.verts[2*i + 2], poly.verts[2*i + 3], poly.verts[2*i + 4], poly.verts[2*i + 5]);
	}
	mdl.vbo = vbo_new(memsz, verts);

	polygon_print(poly);
	if (free_poly)
		polygon_free(poly);
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
