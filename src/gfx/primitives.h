#ifndef GFX_PRIMITIVES_H
#define GFX_PRIMITIVES_H

#define EXPAND_COLOUR(x) x.r, x.g, x.b, x.a

// TODO: Change to vec type
inline static void quad_points(float* points, Rect rect, float z, union Colour colour)
{
	memcpy(points, (float[]){
		rect.x         , rect.y         , z, EXPAND_COLOUR(colour),
		rect.x         , rect.y + rect.h, z, EXPAND_COLOUR(colour),
		rect.x + rect.w, rect.y         , z, EXPAND_COLOUR(colour),
		rect.x + rect.w, rect.y + rect.h, z, EXPAND_COLOUR(colour),
	}, sizeof(float[4*7]));
}

#endif
