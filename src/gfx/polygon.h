#ifndef POLYGON_H
#define POLYGON_H

#include <stdarg.h>

#include "renderer.h"

struct Polygon {
	intptr vertc;
	float* verts;
	vec3   colour;
};

struct Polygon polygon_new(float* verts, vec3 colour);
struct Model polygons_to_model(int polyc, struct Polygon* polys, bool free_polys);
void polygon_print(struct Polygon poly);
void polygon_free(struct Polygon poly);

#endif
