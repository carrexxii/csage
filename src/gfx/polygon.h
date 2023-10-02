#ifndef POLYGON_H
#define POLYGON_H

#include <stdarg.h>

#include "renderer.h"

struct Polygon {
	intptr vertc;
	float* verts;
};

struct Polygon polygon_new(int vertc, ...);
struct Model polygon_to_model(struct Polygon poly, vec3 colour, bool freepoly);
void polygon_print(struct Polygon poly);
void polygon_free(struct Polygon poly);

#endif
