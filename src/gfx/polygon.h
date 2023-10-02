#ifndef POLYGON_H
#define POLYGON_H

#include <stdarg.h>

#include "renderer.h"

struct Polygon {
	intptr vertc;
	float* verts;
};

struct Polygon polygon_new(int vertc, float* verts);
struct Model polygon_to_model(struct Polygon poly, vec3 colour, bool freepoly);
struct Model polygons_to_model(int polyc, struct Polygon* polys, vec3* colours, bool free_polys);
void polygon_print(struct Polygon poly);
void polygon_free(struct Polygon poly);

#endif
