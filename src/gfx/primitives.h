#ifndef GFX_PRIMITIVES_H
#define GFX_PRIMITIVES_H

#include "common.h"
#include "maths/maths.h"

#define EXPAND_COLOUR(x) x.r/255.0f, x.g/255.0f, x.b/255.0f, x.a/255.0f

// TODO: Change to vec type
static inline void quad_from_rect(float* pts, Rect rect, float z, Colour colour)
{
	memcpy(pts, (float[]){
		rect.x         , rect.y         , z, EXPAND_COLOUR(colour),
		rect.x         , rect.y + rect.h, z, EXPAND_COLOUR(colour),
		rect.x + rect.w, rect.y         , z, EXPAND_COLOUR(colour),
		rect.x         , rect.y + rect.h, z, EXPAND_COLOUR(colour),
		rect.x + rect.w, rect.y + rect.h, z, EXPAND_COLOUR(colour),
		rect.x + rect.w, rect.y         , z, EXPAND_COLOUR(colour),
	}, sizeof(float[6*7]));
}

#endif

