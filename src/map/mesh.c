#include "common.h"
#include "gfx/vertices.h"
#include "map.h"

struct Vertex {
	uint8 x, y, z, _;
}; static_assert(sizeof(struct Vertex) == 4, "struct Vertex");

void generate_meshes(struct Map* map)
{
	uint cellc = volume_of(map->dim);

	/* Vertices */
	uint vertc = volume_of((struct Dim){ .w = map->dim.w + 1, .h = map->dim.h + 1, .d = map->dim.d + 1, });
	struct Vertex* verts     = scalloc(vertc, SIZEOF_VXL_VERT);
	struct Vertex* vertstart = verts;
	for (uint z = 0; z < map->dim.d + 1; z++)
		for (uint y = 0; y < map->dim.h + 1; y++)
			for (uint x = 0; x < map->dim.w + 1; x++)
				*verts++ = (struct Vertex){ .x = x, .y = y, .z = z };

	/* Indices */
	uint   indc     = 0;
	uint8* inds     = scalloc(cellc, sizeof(uint8[18])); /* 18 vertices per voxel */
	uint8* indstart = inds;
	uint rowc   =  map->dim.h + 1;               /* # of cells per row     */
	uint layerc = (map->dim.h + 1) * (map->dim.d + 1); /* # of cells per z-level */
	for (uint z = 0; z < map->dim.d; z++) {
		for (uint y = 0; y < map->dim.h; y++) {
			for (uint x = 0; x < map->dim.w; x++) {
				// if (!map->data[i].data)
					// continue;
				/* 2--3   2
				 * | /  / |
				 * 1   1--3
				 */
				/* Top */
				*inds++ = x     + (y + 1)*rowc + z*layerc;
				*inds++ = x + 1 +       y*rowc + z*layerc;
				*inds++ = x     +       y*rowc + z*layerc;

				*inds++ = x     + (y + 1)*rowc + z*layerc;
				*inds++ = x + 1 + (y + 1)*rowc + z*layerc;
				*inds++ = x + 1 +       y*rowc + z*layerc;

				/* Left */
				*inds++ = x + (y + 1)*rowc + (z + 1)*layerc;
				*inds++ = x + (y + 1)*rowc +       z*layerc;
				*inds++ = x +       y*rowc + (z + 1)*layerc;

				*inds++ = x +       y*rowc +       z*layerc;
				*inds++ = x +       y*rowc + (z + 1)*layerc;
				*inds++ = x + (y + 1)*rowc +       z*layerc;

				/* Right */
				*inds++ = x     + y*rowc + (z + 1)*layerc;
				*inds++ = x     + y*rowc +       z*layerc;
				*inds++ = x + 1 + y*rowc +       z*layerc;

				*inds++ = x     + y*rowc + (z + 1)*layerc;
				*inds++ = x + 1 + y*rowc +       z*layerc;
				*inds++ = x + 1 + y*rowc + (z + 1)*layerc;
				// for (uint k = 0; k < 12; k++) inds++;

				indc += 18;
			}
		}
	}

	for (uint v = 0; v < vertc; v++)
		DEBUG(1, "[v%u] %u %u %u", v, vertstart[v].x, vertstart[v].y, vertstart[v].z);
	DEBUG(1, "   ");
	for (uint v = 0; v < indc; v += 3)
		DEBUG(1, "[t%u] %u %u %u", v/3, indstart[v], indstart[v+1], indstart[v+2]);
	map->verts = create_vbo(vertc*SIZEOF_VXL_VERT, vertstart);
	map->inds  = create_ibo(indc*sizeof(uint8), indstart);
	map->indc  = indc;

	free(vertstart);
	free(indstart);
	DEBUG(3, "[MAP] Generated mesh for map with %u cells (%u vertices)", cellc, indc);
	// exit(0);
}
